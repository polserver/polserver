#pragma once

#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/ScopableName.h"
#include "bscript/compiler/model/UserFunctionType.h"

#include <clib/maputil.h>
#include <map>

namespace antlr4
{
class ParserRuleContext;
}

namespace Pol::Bscript::Compiler
{
class Node;

class AvailableSecondPassTarget
{
public:
  enum class Type;

  AvailableSecondPassTarget( const SourceLocation& loc, Type type, Node* context );

  SourceLocation source_location;
  Type type;
  // Used by UserFunctionVisitor/Builder to add var statements to the ClassBody
  // inside (context=) TopLevelStatements; and GeneratedFunctionBuilder to
  // create a SuperFunction for a (context=) ClassDeclaration.
  Node* context;

  enum class Type
  {
    ParseTree,
    GeneratedFunction
  };
};

class AvailableGeneratedFunction : public AvailableSecondPassTarget
{
public:
  AvailableGeneratedFunction( const SourceLocation& loc, Node* context, const ScopableName& name,
                              UserFunctionType type );

  ScopableName name;
  UserFunctionType type;
};

class AvailableParseTree : public AvailableSecondPassTarget
{
public:
  AvailableParseTree( const SourceLocation& loc, antlr4::ParserRuleContext* parse_rule_context,
                      const ScopeName& scope, Node* context );
  antlr4::ParserRuleContext* const parse_rule_context;

  ScopeName scope;
};
}  // namespace Pol::Bscript::Compiler

template <>
struct fmt::formatter<Pol::Bscript::Compiler::AvailableSecondPassTarget>
    : fmt::formatter<std::string>
{
  fmt::format_context::iterator format(
      const Pol::Bscript::Compiler::AvailableSecondPassTarget& apt,
      fmt::format_context& ctx ) const;
};
