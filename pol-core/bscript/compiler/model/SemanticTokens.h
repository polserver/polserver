#ifndef POLSERVER_SEMANTICTOKENS_H
#define POLSERVER_SEMANTICTOKENS_H

#include <cstddef>
#include <vector>

namespace antlr4
{
class Token;
}
namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class ScopeTree;

// FIXME: truncate list after identifying proper usages
enum class SemanticTokenType
{
  NAMESPACE,
  TYPE,
  CLASS,
  ENUM,
  INTERFACE,
  STRUCT,
  TYPEPARAMETER,
  PARAMETER,
  VARIABLE,
  PROPERTY,
  ENUMMEMBER,
  EVENT,
  FUNCTION,
  METHOD,
  MACRO,
  KEYWORD,
  MODIFIER,
  COMMENT,
  STRING,
  NUMBER,
  REGEXP,
  OPERATOR
};

// FIXME: truncate list after identifying proper usages
enum SemanticTokenModifier
{
  DECLARATION,
  DEFINITION,
  READONLY,
  STATIC,
  DEPRECATED,
  ABSTRACT,
  ASYNC,
  MODIFICATION,
  DOCUMENTATION,
  DEFAULTLIBRARY,
};

struct SemanticToken
{
  size_t line_number;
  size_t character_column;
  size_t length;
  SemanticTokenType type;
  std::vector<SemanticTokenModifier> modifiers;

  static void append_from_lexer_token( const antlr4::Token&, std::vector<SemanticToken>& );
};

using SemanticTokens = std::vector<SemanticToken>;
}  // namespace Pol::Bscript::Compiler

#endif
