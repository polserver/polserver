#pragma once

#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/os.h>
#include <fmt/ostream.h>
#include <fstream>
#include <iosfwd>
#include <iterator>
#include <mutex>
#include <stdio.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace Pol::Clib
{
class StreamWriter
{
public:
  StreamWriter( const std::string& path );
  /// A writer with no file behind it: the text piles up in the buffer instead of being written,
  /// for someone else to hand to a real writer with append(). This is how one part of a big file
  /// gets formatted on a worker thread while the parts before it are still being formatted.
  StreamWriter();
  ~StreamWriter() noexcept( false );
  StreamWriter( const StreamWriter& ) = delete;
  StreamWriter& operator=( const StreamWriter& ) = delete;

  template <typename T>
  void add( std::string_view key, T&& value )
  {
    using namespace fmt::literals;
    if constexpr ( !std::is_same<std::decay_t<T>, bool>::value )
      fmt::format_to( std::back_inserter( _mbuff ), "\t{}\t{}\n"_cf, key, value );
    else  // force bool to write as 0/1
      fmt::format_to( std::back_inserter( _mbuff ), "\t{}\t{:d}\n"_cf, key, value );
    maybe_flush();
  }
  /// A value made of two parts, written as "{a} {b}". This is the shape of a CProp line, of which
  /// a large shard writes millions per save: going through add( key, fmt::format( "{} {}", .. ) )
  /// instead allocates and copies a std::string for every one of them.
  template <typename A, typename B>
  void add( std::string_view key, A&& a, B&& b )
  {
    using namespace fmt::literals;
    fmt::format_to( std::back_inserter( _mbuff ), "\t{}\t{} {}\n"_cf, key, a, b );
    maybe_flush();
  }
  /// A value formatted straight into the buffer, for the ones that would otherwise be built in a
  /// temporary first: add_fmt( "Serial", FMT_COMPILE( "{:#x}" ), serial ).
  template <typename S, typename... Args>
  void add_fmt( std::string_view k, S&& formatstr, Args&&... args )
  {
    key( k );
    raw( std::forward<S>( formatstr ), std::forward<Args>( args )... );
    eol();
  }

  /// Piecewise writing, for the properties whose value is assembled conditionally and so cannot
  /// be one format call: key() or key_fmt(), then any number of raw(), then eol().
  void key( std::string_view k )
  {
    using namespace fmt::literals;
    fmt::format_to( std::back_inserter( _mbuff ), "\t{}\t"_cf, k );
  }
  /// For the few properties that number themselves, eg. Spellbits0..7.
  template <typename S, typename... Args>
  void key_fmt( S&& formatstr, Args&&... args )
  {
    _mbuff.push_back( '\t' );
    fmt::format_to( std::back_inserter( _mbuff ), std::forward<S>( formatstr ),
                    std::forward<Args>( args )... );
    _mbuff.push_back( '\t' );
  }
  template <typename S, typename... Args>
  void raw( S&& formatstr, Args&&... args )
  {
    fmt::format_to( std::back_inserter( _mbuff ), std::forward<S>( formatstr ),
                    std::forward<Args>( args )... );
  }
  void eol()
  {
    _mbuff.push_back( '\n' );
    maybe_flush();
  }

  template <typename... Args>
  void comment( std::string_view formatstr, Args&&... args )
  {
    using namespace std::literals;
    _mbuff.append( "# "sv );
    if constexpr ( sizeof...( args ) == 0 )
      _mbuff.append( formatstr );
    else
      fmt::format_to( std::back_inserter( _mbuff ), fmt::runtime( formatstr ), args... );
    _mbuff.push_back( '\n' );
  }
  template <typename Str>
  void begin( Str&& key )
  {
    using namespace fmt::literals;
    fmt::format_to( std::back_inserter( _mbuff ), "{}\n{{\n"_cf, key );
  }
  template <typename Str, typename StrValue>
  void begin( Str&& key, StrValue&& value )
  {
    using namespace fmt::literals;
    fmt::format_to( std::back_inserter( _mbuff ), "{} {}\n{{\n"_cf, key, value );
  }
  void end()
  {
    using namespace std::literals;
    _mbuff.append( "}\n\n"sv );
    maybe_flush();
  }
  void flush_close();

  /// Append an already formatted block, eg. the buffer of a detached writer.
  void append( std::string_view text )
  {
    // A block that already exceeds the buffer would be copied in only to be written straight
    // back out, so hand it to the file directly. Over a save that is one full copy of every
    // byte written, and on a large shard the copying was most of the time the save spent.
    if ( _file != nullptr && text.size() >= FLUSH_THRESHOLD )
    {
      if ( _mbuff.size() )
        flush();  // whatever is buffered belongs in the file ahead of this block
      write_block( text );
      return;
    }
    _mbuff.append( text );
    maybe_flush();
  }
  /// Append a block formatted somewhere else, serialising against other threads appending to
  /// this same file. That is all a file written by many threads at once needs: the blocks arrive
  /// in whatever order the threads finish, but each one arrives whole.
  void append_locked( std::string_view text )
  {
    std::lock_guard<std::mutex> lock( _append_mutex );
    append( text );
  }
  /// What a detached writer has accumulated. Valid until the next write to it.
  std::string_view buffer() const { return { _mbuff.data(), _mbuff.size() }; }
  /// Drop the accumulated text but keep the capacity, so a detached writer can be reused.
  void reset_buffer() { _mbuff.clear(); }

  /// Payload handed to this file so far, the still-buffered tail included.
  size_t bytes_written() const { return _bytes_written + _mbuff.size(); }

protected:
  /// Write the buffer out once it has grown past the threshold. clear() keeps the capacity.
  /// A detached writer has nowhere to write to, so its buffer just keeps growing.
  void maybe_flush()
  {
    if ( _file && _mbuff.size() > FLUSH_THRESHOLD )
      flush();
  }
  void flush();
  /// Write one block straight to the file. Callers are responsible for the buffer being empty.
  void write_block( std::string_view text );

  /// How much text piles up before it is written. stdio buffering is off, so this is the size of
  /// the write() the kernel sees: a megabyte turns the hundreds of thousands of small writes a
  /// large save used to make into a few hundred big ones. Every flush check fires above it, so it
  /// is also where the buffer of a file-backed writer ends up -- see the constructor.
  static constexpr size_t FLUSH_THRESHOLD = 0x100000;

  FILE* _file;
  // formatting creates a temp buffer
  // to prevent this format into this buffer and when full write to disk, clear of the buffer keeps
  // the capacity
  fmt::basic_memory_buffer<char> _mbuff;
  /// Held only by append_locked(), so that the threads formatting one file can hand their blocks
  /// over without stepping on each other. Untaken on every other path.
  std::mutex _append_mutex;
  size_t _bytes_written{ 0 };
};
}  // namespace Pol::Clib
