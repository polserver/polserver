#ifndef CLIB_STREAMSAVER_H
#define CLIB_STREAMSAVER_H

#include <fmt/format.h>
#include <fstream>
#include <iosfwd>
#include <iterator>
#include <string>
#include <type_traits>

#if 0
#include "timer.h"
#endif

namespace Pol
{
namespace Clib
{
class StreamWriter
{
public:
  StreamWriter( std::ofstream* stream );
  ~StreamWriter() = default;
  StreamWriter( const StreamWriter& ) = delete;
  StreamWriter& operator=( const StreamWriter& ) = delete;

  template <typename Str, typename T>
  void add( Str&& key, T&& value )
  {
    if constexpr ( !std::is_same<std::decay_t<T>, bool>::value )  // force bool to write as 0/1
      fmt::print( *_stream, "\t{}\t{}\n", key, value );
    else
      fmt::print( *_stream, "\t{}\t{:d}\n", key, value );
  }
  template <typename Str, typename... Args>
  void comment( Str&& format, Args&&... args )
  {
    *_stream << "# ";
    if constexpr ( sizeof...( args ) == 0 )
      *_stream << format;
    else
      fmt::print( *_stream, format, args... );
    *_stream << '\n';
  }
  template <typename Str>
  void begin( Str&& key )
  {
    fmt::print( *_stream, "{}\n{{\n", key );
  }
  template <typename Str, typename StrValue>
  void begin( Str&& key, StrValue&& value )
  {
    fmt::print( *_stream, "{} {}\n{{\n", key, value );
  }
  void end() { *_stream << "}\n\n"; }
  void init( const std::string& filepath );
  void flush_file();

protected:
  std::ofstream* _stream;
#if 0
      Tools::HighPerfTimer::time_mu _fs_time;
#endif
};

}  // namespace Clib
}  // namespace Pol
#endif  // CLIB_STREAMSAVER_H
