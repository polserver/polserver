#ifndef POLSERVER_USERFUNCTIONBUILDER_H
#define POLSERVER_USERFUNCTIONBUILDER_H

#include "bscript/compiler/astbuilder/CompoundStatementBuilder.h"

namespace Pol::Bscript::Compiler
{
class UserFunction;
class ClassDeclaration;

class UserFunctionBuilder : public CompoundStatementBuilder
{
public:
  UserFunctionBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  std::unique_ptr<UserFunction> function_declaration(
      EscriptGrammar::EscriptParser::FunctionDeclarationContext* );
  std::unique_ptr<UserFunction> function_expression(
      EscriptGrammar::EscriptParser::FunctionExpressionContext* );
  std::unique_ptr<ClassDeclaration> class_declaration(
      EscriptGrammar::EscriptParser::ClassDeclarationContext* );

private:
  template <typename ParserContext>
  std::unique_ptr<UserFunction> make_user_function( const std::string& name, ParserContext* context,
                                                    bool exported,
                                                    antlr4::tree::TerminalNode* end_token );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_USERFUNCTIONBUILDER_H
