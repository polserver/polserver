#ifndef POLSERVER_REPORT_H
#define POLSERVER_REPORT_H


#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class SourceLocation;

class Report
{
public:
  explicit Report( bool display_warnings, bool display_errors = true, bool display_debug = false );
  Report( const Report& ) = delete;
  Report& operator=( const Report& ) = delete;

  template <typename Str, typename... Args>
  inline void error( const SourceLocation& source_location, Str const& format, Args&&... args )
  {
    if ( display_errors )
    {
      auto msg = fmt::format( format, args... );
      report_error( source_location, msg.c_str() );
    }
    else
    {
      ++errors;
    }
  }

  template <typename Str, typename... Args>
  inline void error( const SourceFileIdentifier& ident, Str const& format, Args&&... args )
  {
    SourceLocation loc( &ident, 0, 0 );
    error( loc, format, args... );
  }

  template <typename Str, typename... Args>
  inline void error( const Node& node, Str const& format, Args&&... args )
  {
    error( node.source_location, format, args... );
  }

  // Report.fatal: use this when it's not possible to continue after a user-facing error.
  //
  template <typename Str, typename... Args>
  [[noreturn]] inline void fatal( const SourceLocation& source_location, Str const& format,
                                  Args&&... args )
  {
    auto msg = fmt::format( format, args... );
    report_error( source_location, msg.c_str() );
    throw std::runtime_error( msg.c_str() );
  }

  template <typename Str, typename... Args>
  inline void warning( const SourceLocation& source_location, Str const& format, Args&&... args )
  {
    if ( display_warnings )
    {
      auto msg = fmt::format( format, args... );
      report_warning( source_location, msg.c_str() );
    }
    else
    {
      ++warnings;
    }
  }

  template <typename Str, typename... Args>
  inline void warning( const Node& node, Str const& format, Args&&... args )
  {
    warning( node.source_location, format, args... );
  }

  template <typename Str, typename... Args>
  inline void debug( const SourceLocation& source_location, Str const& format, Args&&... args )
  {
    if ( display_debugs )
    {
      auto msg = fmt::format( format, args... );
      report_debug( source_location, msg.c_str() );
    }
  }

  template <typename Str, typename... Args>
  inline void debug( const Node& node, Str const& format, Args&&... args )
  {
    debug( node.source_location, format, args... );
  }

  [[nodiscard]] unsigned error_count() const;
  [[nodiscard]] unsigned warning_count() const;

  void reset();

private:
  void report_error( const SourceLocation&, const char* msg );
  void report_warning( const SourceLocation&, const char* msg );
  void report_debug( const SourceLocation&, const char* msg );

  const bool display_warnings;
  const bool display_errors;
  const bool display_debugs;
  unsigned errors;
  unsigned warnings;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_REPORT_H
