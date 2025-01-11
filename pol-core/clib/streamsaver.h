#ifndef CLIB_STREAMSAVER_H
#define CLIB_STREAMSAVER_H

#include <fmt/format.h>
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
  ~StreamWriter() noexcept( false );
  StreamWriter( const StreamWriter& ) = delete;
  StreamWriter& operator=( const StreamWriter& ) = delete;

  template <typename Str, typename T>
  void add( Str&& key, T&& value )
  {
    if constexpr ( !std::is_same<std::decay_t<T>, bool>::value )  // force bool to write as 0/1
      fmt::format_to( std::back_inserter( _buf ), "\t{}\t{}\n", key, value );
    else
      fmt::format_to( std::back_inserter( _buf ), "\t{}\t{:d}\n", key, value );
  }
  template <typename Str, typename... Args>
  void comment( Str&& format, Args&&... args )
  {
    _buf += "# ";
    if constexpr ( sizeof...( args ) == 0 )
      _buf += format;
    else
      fmt::format_to( std::back_inserter( _buf ), format, args... );
    _buf += '\n';
  }
  template <typename Str>
  void begin( Str&& key )
  {
    fmt::format_to( std::back_inserter( _buf ), "{}\n{{\n", key );
  }
  template <typename Str, typename StrValue>
  void begin( Str&& key, StrValue&& value )
  {
    fmt::format_to( std::back_inserter( _buf ), "{} {}\n{{\n", key, value );
  }
  void end()
  {
    _buf += "}\n\n";
    flush_test();
  }
  void init( const std::string& filepath );
  void flush();
  void flush_file();
  const std::string& buffer() const { return _buf; };

protected:
  void flush_test();
  std::string _buf = {};
  std::ofstream* _stream;
#if 0
      Tools::HighPerfTimer::time_mu _fs_time;
#endif
  std::string _stream_name;
};

}  // namespace Clib
}  // namespace Pol
#endif  // CLIB_STREAMSAVER_H
