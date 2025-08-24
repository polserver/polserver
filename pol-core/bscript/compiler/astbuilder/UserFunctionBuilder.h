#ifndef POLSERVER_USERFUNCTIONBUILDER_H
#define POLSERVER_USERFUNCTIONBUILDER_H

#include "bscript/compiler/astbuilder/CompoundStatementBuilder.h"

namespace Pol::Bscript::Compiler
{
class UserFunction;
class ClassDeclaration;
class ClassVariableList;
class ClassMethodList;
class Node;

class UserFunctionBuilder : public CompoundStatementBuilder
{
public:
  UserFunctionBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  // Pass empty string as class_name for global static user functions.
  std::unique_ptr<UserFunction> function_declaration(
      EscriptGrammar::EscriptParser::FunctionDeclarationContext*, const std::string& class_name );
  std::unique_ptr<UserFunction> function_expression(
      EscriptGrammar::EscriptParser::FunctionExpressionContext* );
  std::unique_ptr<ClassDeclaration> class_declaration(
      EscriptGrammar::EscriptParser::ClassDeclarationContext*, Node* class_body );

private:
  template <typename FunctionTypeNode, typename ParserContext>
  std::unique_ptr<FunctionTypeNode> make_user_function( const std::string& name,
                                                        ParserContext* context, bool exported,
                                                        const std::string& class_name,
                                                        antlr4::tree::TerminalNode* end_token );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_USERFUNCTIONBUILDER_H
