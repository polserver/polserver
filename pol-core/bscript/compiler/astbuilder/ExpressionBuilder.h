#ifndef POLSERVER_EXPRESSIONBUILDER_H
#define POLSERVER_EXPRESSIONBUILDER_H

#include "compiler/astbuilder/ValueBuilder.h"

#ifndef __TOKENS_H
#include "tokens.h"
#endif

namespace Pol::Bscript::Compiler
{
class Argument;
class ArrayInitializer;
class BinaryOperator;
class DictionaryInitializer;
class ElementAccess;
class Expression;
class FunctionCall;
class UnaryOperator;

class ExpressionBuilder : public ValueBuilder
{
public:
  ExpressionBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  [[noreturn]] static BTokenId unhandled_operator( const SourceLocation& );

  std::unique_ptr<ArrayInitializer> array_initializer(
      EscriptGrammar::EscriptParser::ArrayInitializerContext* );
  std::unique_ptr<ArrayInitializer> array_initializer(
      EscriptGrammar::EscriptParser::BareArrayInitializerContext* );
  std::unique_ptr<ArrayInitializer> array_initializer(
      EscriptGrammar::EscriptParser::ExplicitArrayInitializerContext* );

  std::unique_ptr<BinaryOperator> binary_operator(
      EscriptGrammar::EscriptParser::ExpressionContext* );

  std::unique_ptr<DictionaryInitializer> dictionary_initializer(
      EscriptGrammar::EscriptParser::ExplicitDictInitializerContext* );

  std::unique_ptr<ElementAccess> element_access(
      std::unique_ptr<Expression> lhs, EscriptGrammar::EscriptParser::ExpressionListContext* );

  std::unique_ptr<Expression> expression( EscriptGrammar::EscriptParser::ExpressionContext* );

  std::vector<std::unique_ptr<Expression>> expressions(
      EscriptGrammar::EscriptParser::ArrayInitializerContext* );
  std::vector<std::unique_ptr<Expression>> expressions(
      EscriptGrammar::EscriptParser::ExpressionListContext* );

  std::unique_ptr<FunctionCall> function_call( EscriptGrammar::EscriptParser::FunctionCallContext*,
                                               const std::string& scope );

  std::unique_ptr<Expression> expression_suffix(
      std::unique_ptr<Expression> lhs,
      EscriptGrammar::EscriptParser::ExpressionSuffixContext* );

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
