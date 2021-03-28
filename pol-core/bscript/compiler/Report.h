#ifndef POLSERVER_REPORT_H
#define POLSERVER_REPORT_H

#include <format/format.h>

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class SourceLocation;
class ErrorReporter
{
public:
  virtual ~ErrorReporter(){};
  virtual void report_error( const SourceLocation&, const std::string& msg ) = 0;
  virtual void report_warning( const SourceLocation&, const std::string& msg ) = 0;
  virtual void clear() = 0;
};
class ConsoleReporter : public ErrorReporter
{
public:
  explicit ConsoleReporter( bool display_warnings );
  ConsoleReporter( const ConsoleReporter& ) = delete;
  ConsoleReporter& operator=( const ConsoleReporter& ) = delete;
  void report_error( const SourceLocation&, const std::string& msg ) override;
  void report_warning( const SourceLocation&, const std::string& msg ) override;
  void clear() override;

private:
  const bool display_warnings;
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
  inline void error( const SourceLocation& source_location, Args&&... args )
  {
    fmt::Writer w;
    rec_write( w, std::forward<Args>( args )... );
    report_error( source_location, w.str() );
  }

  template <typename... Args>
  inline void error( const SourceFileIdentifier& ident, Args&&... args )
  {
    SourceLocation loc( &ident, 0, 0 );
    error( loc, args... );
  }

  template <typename... Args>
  inline void error( const Node& node, Args&&... args )
  {
    error( node.source_location, args... );
  }

  // Report.fatal: use this when it's not possible to continue after a user-facing error.
  template <typename... Args>
  [[noreturn]] inline void fatal( const SourceLocation& source_location, Args&&... args )
  {
    fmt::Writer w;
    rec_write( w, std::forward<Args>( args )... );
    report_error( source_location, w.str() );
    throw std::runtime_error( w.str() );
  }

  template <typename... Args>
  inline void warning( const SourceLocation& source_location, Args&&... args )
  {
    fmt::Writer w;
    rec_write( w, std::forward<Args>( args )... );
    report_warning( source_location, w.str() );
  }

  template <typename... Args>
  inline void warning( const Node& node, Args&&... args )
  {
    warning( node.source_location, args... );
  }

  void clear();

  [[nodiscard]] unsigned error_count() const;
  [[nodiscard]] unsigned warning_count() const;

private:
  void report_error( const SourceLocation&, const std::string& msg );
  void report_warning( const SourceLocation&, const std::string& msg );
  ErrorReporter& reporter;

  inline void rec_write( fmt::Writer& /*w*/ ) {}
  template <typename T, typename... Targs>
  inline void rec_write( fmt::Writer& w, T&& value, Targs&&... Fargs )
  {
    try
    {
      w << value;
    } catch(...)
    {
    }
    rec_write( w, std::forward<Targs>( Fargs )... );
  }

  unsigned errors;
  unsigned warnings;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_REPORT_H
