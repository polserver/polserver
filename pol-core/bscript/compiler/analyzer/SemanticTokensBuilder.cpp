#include "SemanticTokensBuilder.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/model/CompilerWorkspace.h"

#include <iostream>

namespace Pol::Bscript::Compiler
{
SemanticTokensBuilder::SemanticTokensBuilder( CompilerWorkspace& workspace, Report& report )
    : workspace( workspace ), report( report )
{
}

void SemanticTokensBuilder::build()
{
  if ( workspace.source )
  {
    workspace.tokens = workspace.source->get_tokens();
  }
}

}  // namespace Pol::Bscript::Compiler
