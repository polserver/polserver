#ifndef POLSERVER_MODULEPROCESSOR_H
#define POLSERVER_MODULEPROCESSOR_H

#include <EscriptGrammar/EscriptParserBaseVisitor.h>

#include <memory>
#include <string>

#include "ModuleDeclarationBuilder.h"

namespace Pol::Bscript::Compiler
{
class Profile;
class Report;
class SourceFile;
class SourceFileIdentifier;
class BuilderWorkspace;

class ModuleProcessor : public EscriptGrammar::EscriptParserBaseVisitor
{
public:
  ModuleProcessor( const SourceFileIdentifier&, BuilderWorkspace&, std::string modulename );

  antlrcpp::Any visitModuleDeclarationStatement(
      EscriptGrammar::EscriptParser::ModuleDeclarationStatementContext* ) override;

private:
  BuilderWorkspace& workspace;

  ModuleDeclarationBuilder tree_builder;
  const std::string modulename;
};

}  // namespace Pol::Bscript::Compiler
#endif  // POLSERVER_MODULEPROCESSOR_HModP
