#ifndef POLSERVER_AVAILABLEUSERFUNCTION_H
#define POLSERVER_AVAILABLEUSERFUNCTION_H

#include "compiler/file/SourceLocation.h"
namespace antlr4
{
class ParserRuleContext;
}

namespace Pol::Bscript::Compiler
{

struct AvailableUserFunction
{
  SourceLocation source_location;
  antlr4::ParserRuleContext* const parse_rule_context;
};


}
}
}
#endif  // POLSERVER_AVAILABLEUSERFUNCTION_H
