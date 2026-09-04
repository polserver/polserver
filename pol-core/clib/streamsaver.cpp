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
  // Held because eviction writes this file from whichever thread ran out of budget, which may not
  // be the thread that owns the part writing to it.
  std::lock_guard<std::mutex> guard( _write_lock );
  // Not write_block: bytes_written() counted this when the file was handed it.
  if ( fwrite( text.data(), sizeof( char ), text.size(), _file ) < text.size() )
    throw std::runtime_error{ "failed to write" };
}

void StreamWriter::write_deferred()
{
  _deferring = false;
  // Taken out of the shared queue in the order they were handed over, which is the order this
  // file has to receive them in.
  std::deque<DeferredBlock> mine;
  {
    std::lock_guard<std::mutex> guard( _deferred_lock );
    for ( auto& block : _deferred )
    {
      if ( block.first != this )
        continue;
      _deferred_total -= block.second.size();
      mine.push_back( std::move( block ) );
    }
    std::erase_if( _deferred,
                   [this]( const DeferredBlock& block ) { return block.first == this; } );
  }
  for ( const auto& block : mine )
    write_raw( block.second );
}

void StreamWriter::flush_close()
{
  if ( !_file )
    return;
  if ( _mbuff.size() )
    flush();
  fclose( _file );
  _file = nullptr;
}

}  // namespace Pol::Clib
