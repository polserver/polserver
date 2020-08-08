#ifndef POLSERVER_REPORT_H
#define POLSERVER_REPORT_H

#include <format/format.h>

#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/file/SourceLocation.h"

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
class SourceLocation;

class Report
{
public:
  explicit Report( bool display_warnings );
  Report( const Report& ) = delete;
  Report& operator=( const Report& ) = delete;

  // Always put a newline at the end of the message.
  template <typename... Args>
  inline void error( const SourceLocation& source_location, Args&&... args )
  {
    fmt::Writer w;
    rec_write( w, std::forward<Args>( args )... );
    report_error( source_location, w.c_str() );
  }

  // Always put a newline at the end of the message.
  template <typename... Args>
  inline void error( const SourceFileIdentifier& ident, Args&&... args )
  {
    SourceLocation loc( &ident, 0, 0 );
    error( loc, args... );
  }

  // Report.fatal: use this when it's not possible to continue after a user-facing error.
  //
  // Always put a newline at the end of the message.
  template <typename... Args>
  [[noreturn]] inline void fatal( const SourceLocation& source_location, Args&&... args )
  {
    fmt::Writer w;
    rec_write( w, std::forward<Args>( args )... );
    report_error( source_location, w.c_str() );
    throw std::runtime_error( w.c_str() );
  }

  // Always put a newline at the end of the message.
  template <typename... Args>
  inline void warning( const SourceLocation& source_location, Args&&... args )
  {
    if ( display_warnings )
    {
      fmt::Writer w;
      rec_write( w, std::forward<Args>( args )... );
      report_warning( source_location, w.c_str() );
    }
  }

  unsigned error_count() const;
  unsigned warning_count() const;

private:
  void report_error( const SourceLocation&, const char* msg );
  void report_warning( const SourceLocation&, const char* msg );

  inline void rec_write( fmt::Writer& /*w*/ ) {}
  template <typename T, typename... Targs>
  inline void rec_write( fmt::Writer& w, T&& value, Targs&&... Fargs )
  {
    w << value;
    rec_write( w, std::forward<Targs>( Fargs )... );
  }

  const bool display_warnings;
  unsigned errors;
  unsigned warnings;
};

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol

#endif  // POLSERVER_REPORT_H
