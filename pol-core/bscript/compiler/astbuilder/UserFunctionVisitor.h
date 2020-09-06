#ifndef POLSERVER_USERFUNCTIONVISITOR_H
#define POLSERVER_USERFUNCTIONVISITOR_H

#include <EscriptGrammar/EscriptParserBaseVisitor.h>

#include "compiler/astbuilder/UserFunctionBuilder.h"

namespace Pol::Bscript::Compiler
{
class Profile;
class Report;
class SourceFileIdentifier;
class BuilderWorkspace;

class UserFunctionVisitor : public EscriptGrammar::EscriptParserBaseVisitor
{
public:
  UserFunctionVisitor( const SourceFileIdentifier&, BuilderWorkspace& );

  antlrcpp::Any visitFunctionDeclaration(
      EscriptGrammar::EscriptParser::FunctionDeclarationContext* ) override;

private:
  BuilderWorkspace& workspace;

  UserFunctionBuilder tree_builder;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_USERFUNCTIONVISITOR_H
