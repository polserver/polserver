#ifndef POLSERVER_VALUEBUILDER_H
#define POLSERVER_VALUEBUILDER_H

#include <stack>

#include "bscript/compiler/astbuilder/TreeBuilder.h"
#include "bscript/compiler/model/ScopeName.h"

#include <EscriptGrammar/EscriptParser.h>

namespace Pol::Bscript::Compiler
{
class BooleanValue;
class IntegerValue;
class FloatValue;
class FunctionExpression;
class FunctionReference;
class StringValue;
class Value;

class ValueBuilder : public TreeBuilder
{
public:
  ValueBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  std::unique_ptr<BooleanValue> bool_value( EscriptGrammar::EscriptParser::BoolLiteralContext* );

  std::unique_ptr<FloatValue> float_value( EscriptGrammar::EscriptParser::FloatLiteralContext* );

  std::unique_ptr<FunctionReference> function_reference(
      EscriptGrammar::EscriptParser::FunctionReferenceContext* );

  std::unique_ptr<FunctionExpression> function_expression(
      EscriptGrammar::EscriptParser::FunctionExpressionContext* );

  std::unique_ptr<IntegerValue> integer_value(
      EscriptGrammar::EscriptParser::IntegerLiteralContext* );

  std::unique_ptr<StringValue> string_value( antlr4::tree::TerminalNode* string_literal,
                                             bool expect_quotes = true );

  std::string unquote( antlr4::tree::TerminalNode* string_literal, bool expect_quotes = true );

  std::unique_ptr<Value> value( EscriptGrammar::EscriptParser::LiteralContext* );

  // Pushed and popped in UserFunctionVisitor.
  //
  // Used when constructing a FunctionCall's `calling_scope`.
  ScopeName current_scope_name;

  // Pushed and popped in UserFunctionVisitor.
  //
  // Needed to determine if a return statement should have a child: constructor
  // functions do not have a child.
  std::stack<bool> in_constructor_function;

private:
  int to_int( EscriptGrammar::EscriptParser::IntegerLiteralContext* );
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_VALUEBUILDER_H
