#pragma once

#include "bscript/compiler/file/SourceLocation.h"

#include <clib/maputil.h>
#include <map>

namespace antlr4
{
class ParserRuleContext;
}

namespace Pol::Bscript::Compiler
{
class Node;

struct AvailableParseTree
{
  SourceLocation source_location;
  antlr4::ParserRuleContext* const parse_rule_context;
  std::string scope;
  // Used by UserFunctionVisitor/Builder to add var statemnts to the ClassBody
  // inside TopLevelStatements.
  Node* top_level_statements_child_node;
};
}  // namespace Pol::Bscript::Compiler
