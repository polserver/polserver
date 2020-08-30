#ifndef POLSERVER_EXPRESSIONBUILDER_H
#define POLSERVER_EXPRESSIONBUILDER_H

#include "compiler/astbuilder/ValueBuilder.h"

#ifndef __TOKENS_H
#include "tokens.h"
#endif

namespace Pol::Bscript::Compiler
{
class Argument;
class Expression;
class FunctionCall;
class GetMember;
class MethodCall;
class UnaryOperator;

class ExpressionBuilder : public ValueBuilder
{
public:
  ExpressionBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  [[noreturn]] static BTokenId unhandled_operator( const SourceLocation& );

  std::unique_ptr<Expression> expression( EscriptGrammar::EscriptParser::ExpressionContext* );

  std::unique_ptr<FunctionCall> function_call( EscriptGrammar::EscriptParser::FunctionCallContext*,
                                               const std::string& scope );

  std::unique_ptr<Expression> prefix_unary_operator(
      EscriptGrammar::EscriptParser::ExpressionContext* );

  std::unique_ptr<Expression> postfix_unary_operator(
      EscriptGrammar::EscriptParser::ExpressionContext* );

  std::unique_ptr<Expression> primary( EscriptGrammar::EscriptParser::PrimaryContext* );

  std::unique_ptr<FunctionCall> scoped_function_call(
      EscriptGrammar::EscriptParser::ScopedFunctionCallContext* );

  std::vector<std::unique_ptr<Argument>> value_arguments(
      EscriptGrammar::EscriptParser::ExpressionListContext* );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_EXPRESSIONBUILDER_H
