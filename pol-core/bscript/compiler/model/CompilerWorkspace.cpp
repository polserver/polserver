#include "CompilerWorkspace.h"

#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/ast/UserFunction.h"
#include "compiler/file/SourceFileIdentifier.h"

namespace Pol::Bscript::Compiler
{
CompilerWorkspace::CompilerWorkspace() = default;

CompilerWorkspace::~CompilerWorkspace() = default;

}  // namespace Pol::Bscript::Compiler
