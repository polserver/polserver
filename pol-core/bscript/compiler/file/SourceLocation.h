#ifndef POLSERVER_SOURCELOCATION_H
#define POLSERVER_SOURCELOCATION_H

#include <fmt/format.h>
#include <string>

namespace antlr4
{
class ParserRuleContext;
class Token;
namespace tree
{
class TerminalNode;
}
}  // namespace antlr4

namespace Pol::Bscript::Compiler
{
class SourceFileIdentifier;

struct Position
{
  size_t line_number;
  size_t character_column;  // 1-based on line, as seen in an editor
  size_t token_index;
};

struct Range
{
  Position start;
  Position end;
  bool contains( const Position& ) const;
  bool contains( const Range& ) const;
  bool contains( size_t line_number, size_t character_column ) const;

  Range( const antlr4::ParserRuleContext& );
  Range( const antlr4::tree::TerminalNode& ctx );
  Range( Position start, Position end );
  Range( const antlr4::Token* token );
};

class SourceLocation
{
public:
  SourceLocation( const SourceFileIdentifier*, size_t line_number, size_t character_column );
  SourceLocation( const SourceFileIdentifier*, const Range& );
  SourceLocation( const SourceFileIdentifier*, const antlr4::ParserRuleContext& );
  SourceLocation( const SourceFileIdentifier*, const antlr4::tree::TerminalNode& );

  SourceLocation( const SourceLocation& ) = default;
  SourceLocation( SourceLocation&& ) = default;
  SourceLocation& operator=( const SourceLocation& ) = delete;
  ~SourceLocation() = default;

  void debug( const std::string& msg ) const;
  std::string getSourceLine() const;

  [[noreturn]] void internal_error( const std::string& msg ) const;
  [[noreturn]] void internal_error( const std::string& msg, const SourceLocation& related ) const;

  // There is a vector of std::unique_ptr<SourceFileIdentifier> that owns
  // the SourceFileIdentifier instances for a given script compilation.
  // If you hold onto a SourceFileIdentifier after that vector goes
  // out of scope, this will be a dangling pointer.
  const SourceFileIdentifier* const source_file_identifier;
  const Range range;
};

}  // namespace Pol::Bscript::Compiler

template <>
struct fmt::formatter<Pol::Bscript::Compiler::SourceLocation> : fmt::formatter<std::string>
{
  fmt::format_context::iterator format( const Pol::Bscript::Compiler::SourceLocation& l,
                                        fmt::format_context& ctx ) const;
};
#endif  // POLSERVER_SOURCELOCATION_H
