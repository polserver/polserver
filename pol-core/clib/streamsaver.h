#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/os.h>
#include <fmt/ostream.h>
#include <fstream>
#include <iosfwd>
#include <iterator>
#include <stdio.h>
#ifdef POL_SAVE_POSITIONED_WRITES
#include <atomic>
#include <cstdint>
#include <mutex>
#endif
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace Pol::Clib
{
/// A string literal usable as a template argument: a structural type, which is what C++20 accepts
/// as a non-type template parameter.
template <size_t N>
struct FixedKey
{
  char text[N]{};
  consteval FixedKey( const char ( &key )[N] ) { std::copy_n( key, N, text ); }
  constexpr std::string_view view() const { return { text, N - 1 }; }
};

/// The whole "\tKey\t" that opens a line, composed once at compile time so that writing it is one
/// copy of a known size.
///
/// Nearly every key in a save is a literal, but handed to a writer as an argument it has to be
/// formatted like any other value - which on a large shard is millions of runtime copies of text
/// that was known when the file was compiled.
template <FixedKey Key>
inline constexpr auto key_prefix = []
{
  std::array<char, Key.view().size() + 2> prefix{};
  prefix[0] = '\t';
  std::copy_n( Key.text, Key.view().size(), prefix.begin() + 1 );
  prefix[prefix.size() - 1] = '\t';
  return prefix;
}();

class StreamWriter
{
public:
  StreamWriter( const std::string& path );
  /// A writer with no file behind it: the text piles up in the buffer instead of being written,
  /// for someone else to hand to a real writer with append(). This is how a worker thread formats
  /// its run of a big file while other threads are formatting runs of the same one.
  StreamWriter();
  ~StreamWriter() noexcept( false );
  StreamWriter( const StreamWriter& ) = delete;
  StreamWriter& operator=( const StreamWriter& ) = delete;

  /// Write "\tKey\tvalue\n" with the key composed at compile time - see key_prefix. This is what
  /// every literal key uses; the string_view overloads below are for the few keys only known at
  /// runtime.
  template <FixedKey Key, typename T>
  void add( T&& value )
  {
    using namespace fmt::literals;
    write_key<Key>();
    if constexpr ( !std::is_same<std::decay_t<T>, bool>::value )
      fmt::format_to( std::back_inserter( _mbuff ), "{}\n"_cf, value );
    else  // force bool to write as 0/1
      fmt::format_to( std::back_inserter( _mbuff ), "{:d}\n"_cf, value );
    maybe_flush();
  }
  /// A value made of two parts, written as "{a} {b}" - the shape of a CProp line.
  template <FixedKey Key, typename A, typename B>
  void add( A&& a, B&& b )
  {
    using namespace fmt::literals;
    write_key<Key>();
    fmt::format_to( std::back_inserter( _mbuff ), "{} {}\n"_cf, a, b );
    maybe_flush();
  }
  /// A hex value, which is most of what add_fmt used to be asked for. One format call for the
  /// whole line rather than a key, a value and a newline written separately.
  template <FixedKey Key, typename T>
  void add_hex( T&& value )
  {
    using namespace fmt::literals;
    write_key<Key>();
    fmt::format_to( std::back_inserter( _mbuff ), "{:#x}\n"_cf, value );
    maybe_flush();
  }
  /// A value formatted straight into the buffer, for the shapes add_hex does not cover:
  /// add_fmt<"Reportable">( "{:#x} {}"_cf, serial, clock ).
  template <FixedKey Key, typename S, typename... Args>
  void add_fmt( S&& formatstr, Args&&... args )
  {
    write_key<Key>();
    raw( std::forward<S>( formatstr ), std::forward<Args>( args )... );
    eol();
  }

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
  /// temporary first: add_fmt( "Serial", "{:#x}"_cf, serial ).
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
  /// What a detached writer has accumulated. Valid until the next write to it.
  std::string_view buffer() const { return { _mbuff.data(), _mbuff.size() }; }
  /// Drop the accumulated text but keep the capacity, so a detached writer can be reused.
  void reset_buffer() { _mbuff.clear(); }

  /// Payload handed to this file so far, the still-buffered tail included.
  size_t bytes_written() const { return _bytes_written + _mbuff.size(); }

#ifdef POL_SAVE_POSITIONED_WRITES
  /// Write this file by reserved offset instead of in sequence, which is what the parallel
  /// section of a save wants: append_at() takes a range of the file for itself and writes it
  /// where it lands, so several threads write to one file with nothing to queue behind.
  ///
  /// Experimental, built only with -DSAVE_POSITIONED_WRITES=ON. It costs the file order, which a
  /// save has already given up, but not where the file ends: end_positioned() puts the stream
  /// back at the end for whoever writes to it in sequence afterwards.
  void begin_positioned();
  /// Reserve a range the size of `text` and write it there. Safe from any number of threads.
  /// A writer with no file behind it has no ranges to hand out and collects the block instead,
  /// exactly as append() does, so a detached writer still works as a destination.
  void append_at( std::string_view text );
  void end_positioned();
#endif

  /// How much text piles up before it is written. stdio buffering is off, so this is the size of
  /// the write() the kernel sees: a megabyte turns the hundreds of thousands of small writes a
  /// large save used to make into a few hundred big ones. Every flush check fires above it, so it
  /// is also where the buffer of a file-backed writer ends up -- see the constructor.
  ///
  /// Public because a block handed to append() at this size goes straight to the file instead of
  /// being copied in, so whoever is filling a detached writer wants to hand it over at exactly
  /// this size and not at a constant of its own.
  static constexpr size_t FLUSH_THRESHOLD = 0x100000;

protected:
  /// The compile-time prefix of a line, copied in one go.
  template <FixedKey Key>
  void write_key()
  {
    constexpr auto& prefix = key_prefix<Key>;
    _mbuff.append( prefix.data(), prefix.data() + prefix.size() );
  }

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

  FILE* _file;
#ifdef POL_SAVE_POSITIONED_WRITES
  /// Where the next reserved range starts. Only meaningful between begin_positioned() and
  /// end_positioned().
  std::atomic<uint64_t> _next_offset{ 0 };
  /// Guards the buffer of a detached writer, which several threads may append to at once because
  /// a positioned save holds nothing else while it hands a block over.
  std::mutex _sink_lock;
#endif
  // formatting creates a temp buffer
  // to prevent this format into this buffer and when full write to disk, clear of the buffer keeps
  // the capacity
  fmt::basic_memory_buffer<char> _mbuff;
#ifdef POL_SAVE_POSITIONED_WRITES
  std::atomic<size_t> _bytes_written{ 0 };  // append_at() counts from several threads at once
#else
  size_t _bytes_written{ 0 };
#endif
};
}  // namespace Pol::Clib
