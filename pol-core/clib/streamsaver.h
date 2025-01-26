#pragma once

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fstream>
#include <iosfwd>
#include <iterator>
#include <memory>
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

  template <typename Str, typename T>
  void add( Str&& key, T&& value )
  {
    if constexpr ( !std::is_same<std::decay_t<T>, bool>::value )  // force bool to write as 0/1
      fmt::print( _stream, "\t{}\t{}\n", key, value );
    else
      fmt::print( _stream, "\t{}\t{:d}\n", key, value );
  }
  template <typename Str, typename... Args>
  void comment( Str&& format, Args&&... args )
  {
    _stream << "# ";
    if constexpr ( sizeof...( args ) == 0 )
      _stream << format;
    else
      fmt::print( _stream, format, args... );
    _stream << '\n';
  }
  template <typename Str>
  void begin( Str&& key )
  {
    fmt::print( _stream, "{}\n{{\n", key );
  }
  template <typename Str, typename StrValue>
  void begin( Str&& key, StrValue&& value )
  {
    fmt::print( _stream, "{} {}\n{{\n", key, value );
  }
  void end() { _stream << "}\n\n"; }
  void open_fstream( const std::string& filepath, std::ofstream& s );
  void flush_file();

protected:
  std::ostream& _stream;
  std::unique_ptr<char[]> _buf;
};

}  // namespace Pol::Clib
