#include "ValueBuilder.h"

#include "compiler/Report.h"
#include "compiler/astbuilder/BuilderWorkspace.h"
#include "compiler/file/SourceLocation.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
ValueBuilder::ValueBuilder( const SourceFileIdentifier& source_file_identifier,
                            BuilderWorkspace& workspace )
    : TreeBuilder( source_file_identifier, workspace )
{
}

}  // namespace Pol::Bscript::Compiler
