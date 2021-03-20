#ifndef POLSERVER_SEMANTICTOKENS_H
#define POLSERVER_SEMANTICTOKENS_H

#include <vector>

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;

// FIXME: truncate list after identifying proper usages
enum SemanticTokenType
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
  OPERATOR,
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
};

using SemanticTokens = std::vector<SemanticToken>;
}  // namespace Pol::Bscript::Compiler

#endif
