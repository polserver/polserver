#pragma once

#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fstream>
#include <iosfwd>
#include <iterator>
#include <string>
#include <type_traits>

namespace Pol::Clib
{
class StreamWriter
{
public:
  StreamWriter( std::ostream& stream );
  ~StreamWriter() noexcept( false );
  StreamWriter( const StreamWriter& ) = delete;
  StreamWriter& operator=( const StreamWriter& ) = delete;

  template <typename T>
  void add( const std::string_view& key, T&& value )
  {
    fmt::format_to( std::back_inserter( _mbuff ), FMT_COMPILE( "\t{}\t" ), key );
    if constexpr ( !std::is_same<std::decay_t<T>, bool>::value )
    {
      // shortcut for strings
      if constexpr ( std::is_same<std::decay_t<T>, std::string>::value ||
                     std::is_same<std::decay_t<T>, std::string_view>::value )
        _mbuff.append( value );
      else
        fmt::format_to( std::back_inserter( _mbuff ), FMT_COMPILE( "{}" ), value );
    }
    else  // force bool to write as 0/1
      fmt::format_to( std::back_inserter( _mbuff ), FMT_COMPILE( "{:d}" ), value );
    _mbuff.push_back( '\n' );
  }
  template <typename... Args>
  void comment( const std::string_view& formatstr, Args&&... args )
  {
    using namespace std::literals;
    _mbuff.append( "# "sv );
    if constexpr ( sizeof...( args ) == 0 )
      _mbuff.append( formatstr );
    else
      fmt::format_to( std::back_inserter( _mbuff ), formatstr, args... );
    _mbuff.push_back( '\n' );
  }
  template <typename Str>
  void begin( Str&& key )
  {
    fmt::format_to( std::back_inserter( _mbuff ), FMT_COMPILE( "{}\n{{\n" ), key );
  }
  template <typename Str, typename StrValue>
  void begin( Str&& key, StrValue&& value )
  {
    fmt::format_to( std::back_inserter( _mbuff ), FMT_COMPILE( "{} {}\n{{\n" ), key, value );
  }
  void end()
  {
    using namespace std::literals;
    _mbuff.append( "}\n\n"sv );
    if ( _mbuff.size() > 10'000 )
    {
      _stream << std::string_view{ _mbuff.data(), _mbuff.size() };
      _mbuff.clear();
    }
  }
  void open_fstream( const std::string& filepath, std::ofstream& s );
  void flush();

protected:
  std::ostream& _stream;
  // formatting creates a temp buffer
  // to prevent this format into this buffer and when full write to disk, clear of the buffer keeps
  // the capacity
  fmt::basic_memory_buffer<char, 10'000> _mbuff;
};

}  // namespace Pol::Clib
