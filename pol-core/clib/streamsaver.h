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
    {
      fmt::format_to( std::back_inserter( _mbuff ), "\t{}\t{}\n", key, value );
    }
    else
      fmt::format_to( std::back_inserter( _mbuff ), "\t{}\t{:d}\n", key, value );
  }
  template <typename Str, typename... Args>
  void comment( Str&& format, Args&&... args )
  {
    static const std::string_view prefix{ "# " };
    _mbuff.append( prefix.data(), prefix.data() + prefix.size() );
    if constexpr ( sizeof...( args ) == 0 )
    {
      const std::string_view strv{ format };
      _mbuff.append( strv.data(), strv.data() + strv.size() );
    }
    else
      fmt::format_to( std::back_inserter( _mbuff ), format, args... );
    _mbuff.push_back( '\n' );
  }
  template <typename Str>
  void begin( Str&& key )
  {
    fmt::format_to( std::back_inserter( _mbuff ), "{}\n{{\n", key );
  }
  template <typename Str, typename StrValue>
  void begin( Str&& key, StrValue&& value )
  {
    fmt::format_to( std::back_inserter( _mbuff ), "{} {}\n{{\n", key, value );
  }
  void end()
  {
    static const std::string_view endv{ "}\n\n" };
    _mbuff.append( endv.data(), endv.data() + endv.size() );
    if ( _mbuff.size() > 5000 )
    {
      _stream << std::string_view{ _mbuff.data(), _mbuff.size() };
      _mbuff.clear();
    }
  }
  void open_fstream( const std::string& filepath, std::ofstream& s );
  void flush_file();

protected:
  std::ostream& _stream;
  // formatting creates a temp buffer
  // to prevent this format into this buffer and when full write to disk, clear of the buffer keeps
  // the capacity
  fmt::basic_memory_buffer<char, 5000> _mbuff;
  // extra buffer for ofstream to be not bound to io speed during write, I think not feasible since
  // buffer has to be quite huge to gain performance
  //  std::unique_ptr<char[]> _buf;
};

}  // namespace Pol::Clib
