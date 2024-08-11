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
struct AvailableParseTree
{
  SourceLocation source_location;
  antlr4::ParserRuleContext* const parse_rule_context;
  std::string scope;
};
}  // namespace Pol::Bscript::Compiler
