#ifndef POLSERVER_REPORT_H
#define POLSERVER_REPORT_H


#include "bscript/compiler/ast/Node.h"
#include "clib/logfacility.h"

namespace Pol::Bscript::Compiler
{
class SourceLocation;
class ErrorReporter
{
public:
  virtual ~ErrorReporter(){};
  virtual void report_error( const SourceLocation&, const std::string& msg ) = 0;
  virtual void report_warning( const SourceLocation&, const std::string& msg ) = 0;
  virtual void report_debug( const SourceLocation&, const std::string& msg ) = 0;
  virtual void clear() = 0;
};
class ConsoleReporter : public ErrorReporter
{
public:
  explicit ConsoleReporter( bool display_warnings, bool display_errors = true,
                            bool display_debugs = false );
  ConsoleReporter( const ConsoleReporter& ) = delete;
  ConsoleReporter& operator=( const ConsoleReporter& ) = delete;
  void report_error( const SourceLocation&, const std::string& msg ) override;
  void report_warning( const SourceLocation&, const std::string& msg ) override;
  void report_debug( const SourceLocation&, const std::string& msg ) override;
  void clear() override;

private:
  const bool display_warnings;
  const bool display_errors;
  const bool display_debugs;
};
struct Diagnostic
{
  enum class Severity
  {
    Warning,
    Error
  } severity;
  SourceLocation location;
  std::string message;
};
class DiagnosticReporter : public ErrorReporter
{
public:
  explicit DiagnosticReporter() = default;
  DiagnosticReporter( const DiagnosticReporter& ) = delete;
  DiagnosticReporter& operator=( const DiagnosticReporter& ) = delete;
  void report_error( const SourceLocation&, const std::string& ) override;
  void report_warning( const SourceLocation&, const std::string& ) override;
  void report_debug( const SourceLocation&, const std::string& ) override;
  void clear() override;

  std::vector<Diagnostic> diagnostics;
};

class Report
{
public:
  explicit Report( ErrorReporter& report );
  Report( const Report& ) = delete;
  Report& operator=( const Report& ) = delete;

  template <typename... Args>
  inline void error( const SourceLocation& source_location, fmt::format_string<Args...>&& format,
                     Args&&... args )
  {
    ++errors;
    report_error( source_location, fmt::format( format, std::forward<Args>( args )... ) );
  }

  inline void error( const SourceLocation& source_location, const std::string& msg )
  {
    ++errors;
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
    report_debug( source_location, fmt::format( format, std::forward<Args>( args )... ) );
  }

  template <typename... Args>
  inline void debug( const Node& node, fmt::format_string<Args...>&& format, Args&&... args )
  {
    debug( node.source_location, std::forward<fmt::format_string<Args...>>( format ),
           std::forward<Args>( args )... );
  }

  void clear();

  [[nodiscard]] unsigned error_count() const;
  [[nodiscard]] unsigned warning_count() const;

  void reset();

private:
  void report_error( const SourceLocation& source_location, const std::string& msg ) const;
  void report_warning( const SourceLocation& source_location, const std::string& msg ) const;
  void report_debug( const SourceLocation& source_location, const std::string& msg ) const;
  ErrorReporter& reporter;

  unsigned errors;
  unsigned warnings;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_REPORT_H
