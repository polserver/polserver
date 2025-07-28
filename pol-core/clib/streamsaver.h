#pragma once

#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/os.h>
#include <fmt/ostream.h>
#include <fstream>
#include <iosfwd>
#include <iterator>
#include <stdio.h>
#include <string>
#include <type_traits>

namespace Pol::Clib
{
class StreamWriter
{
public:
  StreamWriter( const std::string& path );
  ~StreamWriter() noexcept( false );
  StreamWriter( const StreamWriter& ) = delete;
  StreamWriter& operator=( const StreamWriter& ) = delete;

  template <typename T>
  void add( const std::string_view& key, T&& value )
  {
    using namespace fmt::literals;
    if constexpr ( !std::is_same<std::decay_t<T>, bool>::value )
      fmt::format_to( std::back_inserter( _mbuff ), "\t{}\t{}\n"_cf, key, value );
    else  // force bool to write as 0/1
      fmt::format_to( std::back_inserter( _mbuff ), "\t{}\t{:d}\n"_cf, key, value );
  }
  template <typename... Args>
  void comment( const std::string_view& formatstr, Args&&... args )
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
    if ( _mbuff.size() > 0x8000 )
    {
      auto size = fwrite( _mbuff.data(), sizeof( char ), _mbuff.size(), _file );
      if ( size < _mbuff.size() )
        throw std::runtime_error{ "failed to write" };
      _mbuff.clear();
    }
  }
  void flush_close();

protected:
  FILE* _file;
  // formatting creates a temp buffer
  // to prevent this format into this buffer and when full write to disk, clear of the buffer keeps
  // the capacity
  fmt::basic_memory_buffer<char, 0x8000> _mbuff;
};
}  // namespace Pol::Clib
