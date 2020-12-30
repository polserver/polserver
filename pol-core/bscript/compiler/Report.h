#ifndef POLSERVER_REPORT_H
#define POLSERVER_REPORT_H

#include <format/format.h>

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
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

  // Always put a newline at the end of the message.
  template <typename... Args>
  inline void error( const Node& node, Args&&... args )
  {
    error( node.source_location, args... );
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

  // Always put a newline at the end of the message.
  template <typename... Args>
  inline void warning( const Node& node, Args&&... args )
  {
    warning( node.source_location, args... );
  }

  [[nodiscard]] unsigned error_count() const;
  [[nodiscard]] unsigned warning_count() const;

protected:
  virtual void report_error( const SourceLocation&, const char* msg );
  virtual void report_warning( const SourceLocation&, const char* msg );

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

  const bool display_warnings;
  unsigned errors;
  unsigned warnings;
};

struct Diagnostic
{
  std::string file;
  unsigned line;
  unsigned character;
  enum class DiagnosticSeverity
  {
    Warning,
    Error
  } severity;
  std::string message;
};
class DiagnosticReport : public Report
{
public:
  explicit DiagnosticReport() : Report( true ){};
  DiagnosticReport( const DiagnosticReport& ) = delete;
  DiagnosticReport& operator=( const DiagnosticReport& ) = delete;

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

  // Always put a newline at the end of the message.
  template <typename... Args>
  inline void error( const Node& node, Args&&... args )
  {
    error( node.source_location, args... );
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
    fmt::Writer w;
    rec_write( w, std::forward<Args>( args )... );
    report_warning( source_location, w.c_str() );
  }

  // Always put a newline at the end of the message.
  template <typename... Args>
  inline void warning( const Node& node, Args&&... args )
  {
    warning( node.source_location, args... );
  }

  std::vector<Diagnostic> diagnostics;

private:
  void report_error( const SourceLocation&, const char* msg ) override;
  void report_warning( const SourceLocation&, const char* msg ) override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_REPORT_H
