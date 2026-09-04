#include <chrono>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>

#include "clib/streamsaver.h"

#ifdef POL_SAVE_POSITIONED_WRITES
#include <algorithm>
#include <cerrno>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif
#endif

namespace Pol::Clib
{
StreamWriter::StreamWriter( const std::string& path ) : _file( fopen( path.c_str(), "wb+" ) )
{
  if ( !_file )
    throw std::runtime_error{ fmt::format( "failed to open {}", path ) };
  setbuf( _file, nullptr );  // disable buffer
  // Every flush check fires above the threshold, so this is where the buffer ends up regardless.
  // Taking it in one allocation instead of growing into it saves a handful of reallocate-and-copy
  // rounds per file. A detached writer gets no reserve: it never flushes, and grows to whatever
  // one chunk needs.
  _mbuff.reserve( FLUSH_THRESHOLD );
}

StreamWriter::StreamWriter() : _file( nullptr ) {}

StreamWriter::~StreamWriter() noexcept( false )
{
  auto stack_unwinding = std::uncaught_exceptions();
  try
  {
    flush_close();
  }
  catch ( ... )
  {
    // during stack unwinding an exception would terminate
    if ( !stack_unwinding )
      throw;
  }
}

void StreamWriter::write_block( std::string_view text )
{
  if ( text.empty() )
    return;
  auto size = fwrite( text.data(), sizeof( char ), text.size(), _file );
  if ( size < text.size() )
    throw std::runtime_error{ "failed to write" };
  _bytes_written += text.size();
}

void StreamWriter::flush()
{
  if ( _deferring && _file != nullptr && _mbuff.size() )
  {
    // One path to the file: written straight out, a buffered tail would land ahead of blocks
    // still queued for this file. write_gotten_items() fills items.txt this way while the parts
    // that share it are still being drained.
    enqueue( std::string( _mbuff.data(), _mbuff.size() ) );
    _mbuff.clear();
    return;
  }
  write_block( { _mbuff.data(), _mbuff.size() } );
  _mbuff.clear();
}

#ifdef POL_SAVE_POSITIONED_WRITES
void StreamWriter::begin_positioned()
{
  if ( !_file )
    return;
  if ( _mbuff.size() )
    flush();  // whatever is buffered belongs in the file ahead of any reserved range
#ifdef _WIN32
  const auto at = _ftelli64( _file );
#else
  const auto at = ftello( _file );
#endif
  if ( at < 0 )
    throw std::runtime_error{ "failed to find the end of the file" };
  _next_offset = static_cast<uint64_t>( at );
}

void StreamWriter::append_at( std::string_view text )
{
  if ( text.empty() )
    return;
  if ( !_file )
  {
    // Nowhere to reserve a range: collect the block the way append() would. Which block lands
    // first is as arbitrary as it is in a file, but the appends themselves are one at a time.
    std::lock_guard<std::mutex> guard( _sink_lock );
    _mbuff.append( text );
    return;
  }
  // The range is this thread's alone, so the write needs nothing held while it happens.
  uint64_t at = _next_offset.fetch_add( text.size() );
  const char* data = text.data();
  size_t left = text.size();
  while ( left != 0 )
  {
#ifdef _WIN32
    OVERLAPPED where{};
    where.Offset = static_cast<DWORD>( at & 0xFFFFFFFFu );
    where.OffsetHigh = static_cast<DWORD>( at >> 32 );
    DWORD wrote = 0;
    const DWORD ask = static_cast<DWORD>( std::min<size_t>( left, 0x40000000 ) );
    const HANDLE file = reinterpret_cast<HANDLE>( _get_osfhandle( _fileno( _file ) ) );
    if ( !WriteFile( file, data, ask, &wrote, &where ) )
      throw std::runtime_error{ "failed to write" };
#else
    const ssize_t wrote = pwrite( fileno( _file ), data, left, static_cast<off_t>( at ) );
    if ( wrote < 0 )
    {
      if ( errno == EINTR )
        continue;
      throw std::runtime_error{ "failed to write" };
    }
#endif
    data += wrote;
    left -= static_cast<size_t>( wrote );
    at += static_cast<uint64_t>( wrote );
  }
  _bytes_written += text.size();
}

void StreamWriter::end_positioned()
{
  if ( !_file )
    return;
  // A positioned write leaves the stream where it was, so put it at the end for whatever writes
  // next in sequence - items.txt takes its gotten items after the parallel section.
  const auto end = static_cast<int64_t>( _next_offset.load() );
#ifdef _WIN32
  if ( _fseeki64( _file, end, SEEK_SET ) != 0 )
#else
  if ( fseeko( _file, static_cast<off_t>( end ), SEEK_SET ) != 0 )
#endif
    throw std::runtime_error{ "failed to seek to the end of the file" };
}
#endif

void StreamWriter::write_raw( std::string_view text )
{
  if ( text.empty() || _file == nullptr )
    return;
  // Not write_block: bytes_written() counted this when the file was handed it.
  if ( fwrite( text.data(), sizeof( char ), text.size(), _file ) < text.size() )
    throw std::runtime_error{ "failed to write" };
}

void StreamWriter::enqueue( std::string&& text )
{
  const size_t size = text.size();
  {
    std::unique_lock<std::mutex> guard( _deferred_lock );
    // A block bigger than the whole budget still has to go somewhere, so what is waited for is
    // room beside what is already held, never an empty queue.
    const auto room = [size]
    { return _deferred_total == 0 || _deferred_total + size <= _deferred_budget; };
    if ( !room() )
    {
      // Timed because this is the one thing that can reach back into the stopped window: a thread
      // waiting here is a thread that would otherwise be formatting.
      const auto since = std::chrono::steady_clock::now();
      _deferred_room.wait( guard, room );
      _deferred_stall_us += std::chrono::duration_cast<std::chrono::microseconds>(
                                std::chrono::steady_clock::now() - since )
                                .count();
    }
    _deferred_total += size;
  }
  _bytes_written += size;  // the file has been handed it, whatever is done with it
  _pending.push_move( std::move( text ) );
}

void StreamWriter::release_bytes( size_t size )
{
  {
    std::lock_guard<std::mutex> guard( _deferred_lock );
    _deferred_total -= size;
  }
  _deferred_room.notify_all();
}

void StreamWriter::write_blocks( std::list<std::string>& blocks )
{
  for ( auto& block : blocks )
  {
    if ( !_writer_error )
    {
      try
      {
        write_raw( block );
      }
      catch ( ... )
      {
        _writer_error = std::current_exception();
      }
    }
    // Given back whether or not it was written: a thread waiting for room must not be left
    // waiting because this file has failed.
    release_bytes( block.size() );
  }
  blocks.clear();
}

void StreamWriter::writer_loop()
{
  std::list<std::string> blocks;
  for ( ;; )
  {
    try
    {
      _pending.pop_wait( &blocks );
    }
    catch ( const message_queue<std::string>::Canceled& )
    {
      // write_deferred() cancels once nothing more can be queued. pop_wait() abandons whatever is
      // still in the queue when it throws, so the tail of the save is taken out by hand - it is
      // the end of this file, not text to drop.
      _pending.pop_remaining( &blocks );
      write_blocks( blocks );
      return;
    }
    write_blocks( blocks );
  }
}

void StreamWriter::defer_writes()
{
  if ( _file == nullptr || _deferring )
    return;
  _deferring = true;
  _writer_error = nullptr;
  _writer = std::thread( [this]() { writer_loop(); } );
}

void StreamWriter::write_deferred()
{
  if ( !_deferring )
    return;
  _deferring = false;  // set before the cancel, so nothing can be queued after it
  _pending.cancel();
  if ( _writer.joinable() )
    _writer.join();
  if ( _writer_error )
  {
    auto error = _writer_error;
    _writer_error = nullptr;
    std::rethrow_exception( error );
  }
}

void StreamWriter::flush_close()
{
  if ( !_file )
    return;
  write_deferred();  // nothing may be left with this file's writer when the file is closed
  if ( _mbuff.size() )
    flush();
  fclose( _file );
  _file = nullptr;
}

}  // namespace Pol::Clib
