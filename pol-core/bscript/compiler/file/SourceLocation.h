#ifndef POLSERVER_SOURCELOCATION_H
#define POLSERVER_SOURCELOCATION_H

#include <string>

#include "clib/formatfwd.h"

namespace antlr4
{
class ParserRuleContext;
namespace tree
{
class TerminalNode;
}
}

namespace Pol::Bscript::Compiler
{
class SourceFileIdentifier;

class SourceLocation
{
public:
  SourceLocation( const SourceFileIdentifier*, unsigned short line_number,
                  unsigned short character_column );
  SourceLocation( const SourceFileIdentifier*, antlr4::ParserRuleContext& );
  SourceLocation( const SourceFileIdentifier*, antlr4::tree::TerminalNode& );

  SourceLocation( const SourceLocation& ) = default;
  SourceLocation( SourceLocation&& ) = default;
  SourceLocation& operator=( const SourceLocation& ) = delete;
  ~SourceLocation() = default;

  void debug( const std::string& msg ) const;

  [[noreturn]] void internal_error( const std::string& msg ) const;
  [[noreturn]] void internal_error( const std::string& msg, const SourceLocation& related ) const;

  // There is a vector of std::unique_ptr<SourceFileIdentifier> that owns
  // the SourceFileIdentifier instances for a given script compilation.
  // If you hold onto a SourceFileIdentifier after that vector goes
  // out of scope, this will be a dangling pointer.
  const SourceFileIdentifier* const source_file_identifier;
  const unsigned short line_number;
  const unsigned short character_column; // 1-based on line, as seen in an editor
};

fmt::Writer& operator<<( fmt::Writer&, const SourceLocation& );  // pathname:line:column

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SOURCELOCATION_H
