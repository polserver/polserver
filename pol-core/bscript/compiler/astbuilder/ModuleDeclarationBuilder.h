#ifndef POLSERVER_MODULEDECLARATIONBUILDER_H
#define POLSERVER_MODULEDECLARATIONBUILDER_H

#include "compiler/astbuilder/SimpleStatementBuilder.h"

namespace Pol::Bscript::Compiler
{

class ModuleDeclarationBuilder : public SimpleStatementBuilder
{
public:
  ModuleDeclarationBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_MODULEDECLARATIONBUILDER_H
