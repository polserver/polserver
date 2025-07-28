#ifndef POLSERVER_REPORT_H
#define POLSERVER_REPORT_H


#include "bscript/compiler/ast/Node.h"
#include "clib/logfacility.h"

namespace Pol::Bscript::Compiler
{
class SourceLocation;

class Report
{
public:
  explicit Report( bool display_warnings, bool display_errors = true, bool display_debug = false );
  Report( const Report& ) = delete;
  Report& operator=( const Report& ) = delete;

  template <typename... Args>
  inline void error( const SourceLocation& source_location, fmt::format_string<Args...>&& format,
                     Args&&... args )
  {
    ++errors;
    if ( display_errors )
      report_error( source_location, fmt::format( format, std::forward<Args>( args )... ) );
  }

  inline void error( const SourceLocation& source_location, const std::string& msg )
  {
    ++errors;
    if ( display_errors )
      report_error( source_location, msg );
  }

  template <typename... Args>
  inline void error( const SourceFileIdentifier& ident, fmt::format_string<Args...>&& format,
                     Args&&... args )
  {
    SourceLocation loc( &ident, 0, 0 );
    error( loc, std::forward<fmt::format_string<Args...>>( format ),
           std::forward<Args>( args )... );
  }

  template <typename... Args>
  inline void error( const Node& node, fmt::format_string<Args...>&& format, Args&&... args )
  {
    error( node.source_location, std::forward<fmt::format_string<Args...>>( format ),
           std::forward<Args>( args )... );
  }

  inline void error( const Node& node, const std::string& msg )
  {
    error( node.source_location, msg );
  }

  // Report.fatal: use this when it's not possible to continue after a user-facing error.
  //
  template <typename... Args>
  [[noreturn]] inline void fatal( const SourceLocation& source_location,
                                  fmt::format_string<Args...>&& format, Args&&... args )
  {
    auto msg = fmt::format( format, std::forward<Args>( args )... );
    report_error( source_location, msg );
    throw std::runtime_error( msg );
  }

  template <typename... Args>
  inline void warning( const SourceLocation& source_location, fmt::format_string<Args...>&& format,
                       Args&&... args )
  {
    ++warnings;
    if ( display_warnings )
      report_warning( source_location, fmt::format( format, std::forward<Args>( args )... ) );
  }

  template <typename... Args>
  inline void warning( const Node& node, fmt::format_string<Args...>&& format, Args&&... args )
  {
    warning( node.source_location, std::forward<fmt::format_string<Args...>>( format ),
             std::forward<Args>( args )... );
  }

  template <typename... Args>
  inline void debug( const SourceLocation& source_location, fmt::format_string<Args...>&& format,
                     Args&&... args )
  {
    if ( display_debugs )
      report_debug( source_location, fmt::format( format, std::forward<Args>( args )... ) );
  }

  template <typename... Args>
  inline void debug( const Node& node, fmt::format_string<Args...>&& format, Args&&... args )
  {
    debug( node.source_location, std::forward<fmt::format_string<Args...>>( format ),
           std::forward<Args>( args )... );
  }

  [[nodiscard]] unsigned error_count() const;
  [[nodiscard]] unsigned warning_count() const;

  void reset();

private:
  void report_error( const SourceLocation& source_location, const std::string& msg ) const;
  void report_warning( const SourceLocation& source_location, const std::string& msg ) const;
  void report_debug( const SourceLocation& source_location, const std::string& msg ) const;
  const bool display_warnings;
  const bool display_errors;
  const bool display_debugs;
  unsigned errors;
  unsigned warnings;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_REPORT_H
