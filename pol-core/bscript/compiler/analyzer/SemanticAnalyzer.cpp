#include "SemanticAnalyzer.h"

#include "compiler/Report.h"
#include "compiler/model/CompilerWorkspace.h"

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
SemanticAnalyzer::SemanticAnalyzer( Report& report )
  : report( report )
{
}

SemanticAnalyzer::~SemanticAnalyzer() = default;

void SemanticAnalyzer::register_const_declarations( CompilerWorkspace& /*workspace*/ )
{
}

void SemanticAnalyzer::analyze( CompilerWorkspace& /*workspace*/ )
{
}

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol
