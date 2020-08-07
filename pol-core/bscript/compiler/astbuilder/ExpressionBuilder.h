#ifndef POLSERVER_EXPRESSIONBUILDER_H
#define POLSERVER_EXPRESSIONBUILDER_H

#include "ValueBuilder.h"

namespace Pol::Bscript::Compiler
{
class Expression;
class Argument;
class Expression;
class FunctionCall;

class ExpressionBuilder : public ValueBuilder
{
public:
  ExpressionBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  std::unique_ptr<Expression> additive_expression(
      EscriptGrammar::EscriptParser::AdditiveExpressionContext* );

  std::unique_ptr<Expression> atomic( EscriptGrammar::EscriptParser::AtomicExpressionContext* );

  std::unique_ptr<Expression> bitshift_left( EscriptGrammar::EscriptParser::BitshiftLeftContext* );
  std::unique_ptr<Expression> bitshift_right(
      EscriptGrammar::EscriptParser::BitshiftRightContext* );
  std::unique_ptr<Expression> bitwise_conjunction(
      EscriptGrammar::EscriptParser::BitwiseConjunctionContext* );
  std::unique_ptr<Expression> bitwise_disjunction(
      EscriptGrammar::EscriptParser::BitwiseDisjunctionContext* );
  std::unique_ptr<Expression> bitwise_xor( EscriptGrammar::EscriptParser::BitwiseXorContext* );

  std::unique_ptr<Expression> comparison( EscriptGrammar::EscriptParser::ComparisonContext* );

  std::unique_ptr<Expression> conjunction( EscriptGrammar::EscriptParser::ConjunctionContext* );

  std::unique_ptr<Expression> disjunction( EscriptGrammar::EscriptParser::DisjunctionContext* );

  std::unique_ptr<Expression> elvis_expression(
      EscriptGrammar::EscriptParser::ElvisExpressionContext* );

  std::unique_ptr<Expression> equality( EscriptGrammar::EscriptParser::EqualityContext* );

  std::unique_ptr<Expression> expression( EscriptGrammar::EscriptParser::ExpressionContext* );
  std::unique_ptr<Expression> expression(
      EscriptGrammar::EscriptParser::UnambiguousExpressionContext* );

  std::unique_ptr<FunctionCall> function_call( EscriptGrammar::EscriptParser::MethodCallContext*,
                                               const std::string& scope );
  std::unique_ptr<FunctionCall> function_call(
      EscriptGrammar::EscriptParser::UnambiguousFunctionCallContext*, const std::string& scope );

  std::unique_ptr<Expression> infix_operation(
      EscriptGrammar::EscriptParser::InfixOperationContext* );

  std::unique_ptr<Expression> membership( EscriptGrammar::EscriptParser::MembershipContext* );

  std::unique_ptr<Expression> multiplicative_expression(
      EscriptGrammar::EscriptParser::MultiplicativeExpressionContext* );

  std::unique_ptr<Expression> prefix_unary_arithmetic(
      EscriptGrammar::EscriptParser::PrefixUnaryArithmeticExpressionContext* );

  std::unique_ptr<Expression> prefix_unary_inversion(
      EscriptGrammar::EscriptParser::PrefixUnaryInversionExpressionContext* );

  std::unique_ptr<Expression> prefix_unary_operator(
      EscriptGrammar::EscriptParser::ExpressionContext* );

  std::unique_ptr<Expression> postfix_unary_operator(
      EscriptGrammar::EscriptParser::ExpressionContext* );

  std::unique_ptr<Expression> postfix_unary(
      EscriptGrammar::EscriptParser::PostfixUnaryExpressionContext* );

  std::unique_ptr<Expression> primary( EscriptGrammar::EscriptParser::PrimaryContext* );

  std::unique_ptr<FunctionCall> scoped_function_call(
      EscriptGrammar::EscriptParser::ScopedMethodCallContext* );
  std::unique_ptr<FunctionCall> scoped_function_call(
      EscriptGrammar::EscriptParser::UnambiguousScopedFunctionCallContext* );

  std::vector<std::unique_ptr<Argument>> value_arguments(
      EscriptGrammar::EscriptParser::ValueArgumentsContext* ctx );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_EXPRESSIONBUILDER_H
