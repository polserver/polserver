#ifndef POLSERVER_USERFUNCTIONVISITOR_H
#define POLSERVER_USERFUNCTIONVISITOR_H

#include <EscriptGrammar/EscriptParserBaseVisitor.h>

#include "bscript/compiler/astbuilder/UserFunctionBuilder.h"

namespace Pol::Bscript::Compiler
{
class Profile;
class Report;
class SourceFileIdentifier;
class BuilderWorkspace;
class Node;

class UserFunctionVisitor : public EscriptGrammar::EscriptParserBaseVisitor
{
public:
  UserFunctionVisitor( const SourceFileIdentifier&, BuilderWorkspace&, const std::string& scope,
                       Node* top_level_statements_child_node );

  antlrcpp::Any visitFunctionDeclaration(
      EscriptGrammar::EscriptParser::FunctionDeclarationContext* ) override;

  antlrcpp::Any visitFunctionExpression(
      EscriptGrammar::EscriptParser::FunctionExpressionContext* ) override;

  antlrcpp::Any visitClassDeclaration(
      EscriptGrammar::EscriptParser::ClassDeclarationContext* ) override;

private:
  BuilderWorkspace& workspace;

  UserFunctionBuilder tree_builder;
  const std::string scope;
  Node* top_level_statements_child_node;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_USERFUNCTIONVISITOR_H
