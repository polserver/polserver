#ifndef POLSERVER_MODULEDECLARATIONBUILDER_H
#define POLSERVER_MODULEDECLARATIONBUILDER_H

#include "bscript/compiler/astbuilder/SimpleStatementBuilder.h"

namespace Pol::Bscript::Compiler
{
class ModuleFunctionDeclaration;

class ModuleDeclarationBuilder : public SimpleStatementBuilder
{
public:
  ModuleDeclarationBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  std::unique_ptr<ModuleFunctionDeclaration> module_function_declaration(
      EscriptGrammar::EscriptParser::ModuleFunctionDeclarationContext*, std::string module_name );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_MODULEDECLARATIONBUILDER_H
