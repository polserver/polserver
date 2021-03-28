#ifndef POLSERVER_SEMANTICTOKENS_H
#define POLSERVER_SEMANTICTOKENS_H

#include <list>
#include <memory>
#include <vector>

namespace antlr4
{
class Token;
}
namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;

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

class SemanticTokenType2
{
public:
  enum Value : uint8_t
  {
    Apple,
    Pear,
    Banana,
    Strawberry
  };
};

// void foo() {
//   // SemanticTokenType2::Apple;
// }
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
  std::list<SemanticTokenModifier> modifiers;

  static std::unique_ptr<std::list<SemanticToken>> from_lexer_token( const antlr4::Token& );
};

using SemanticTokens = std::list<SemanticToken>;
}  // namespace Pol::Bscript::Compiler

#endif
