#ifndef POLSERVER_EXPRESSIONBUILDER_H
#define POLSERVER_EXPRESSIONBUILDER_H

#include "compiler/astbuilder/ValueBuilder.h"

namespace Pol::Bscript::Compiler
{
class Argument;
class Expression;
class FunctionCall;

class ExpressionBuilder : public ValueBuilder
{
public:
  ExpressionBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  std::unique_ptr<Expression> expression( EscriptGrammar::EscriptParser::ExpressionContext* );

  std::unique_ptr<FunctionCall> function_call( EscriptGrammar::EscriptParser::FunctionCallContext*,
                                               const std::string& scope );

  std::unique_ptr<Expression> primary( EscriptGrammar::EscriptParser::PrimaryContext* );

  std::unique_ptr<FunctionCall> scoped_function_call(
      EscriptGrammar::EscriptParser::ScopedFunctionCallContext* );

  std::vector<std::unique_ptr<Argument>> value_arguments(
      EscriptGrammar::EscriptParser::ExpressionListContext* );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_EXPRESSIONBUILDER_H
