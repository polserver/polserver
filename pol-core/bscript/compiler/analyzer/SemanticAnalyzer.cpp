#include "SemanticAnalyzer.h"

#include "compiler/Report.h"
#include "compiler/model/CompilerWorkspace.h"

namespace Pol::Bscript::Compiler
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

}  // namespace Pol::Bscript::Compiler
