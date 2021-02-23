#ifndef POLSERVER_VALUEBUILDER_H
#define POLSERVER_VALUEBUILDER_H

#include "compiler/astbuilder/TreeBuilder.h"

#include <EscriptGrammar/EscriptParser.h>

namespace Pol::Bscript::Compiler
{
class IntegerValue;
class InterpolatedStringValue;
class FloatValue;
class FunctionReference;
class StringValue;
class Value;

class ValueBuilder : public TreeBuilder
{
public:
  ValueBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  std::unique_ptr<FloatValue> float_value(
      EscriptGrammar::EscriptParser::FloatLiteralContext* );

  std::unique_ptr<FunctionReference> function_reference(
      EscriptGrammar::EscriptParser::FunctionReferenceContext* );

  std::unique_ptr<IntegerValue> integer_value(
      EscriptGrammar::EscriptParser::IntegerLiteralContext* );

  std::unique_ptr<StringValue> regular_string_value( antlr4::tree::TerminalNode* regular_string );

  std::unique_ptr<InterpolatedStringValue> interpolated_string_value(
      EscriptGrammar::EscriptParser::InterpolatedStringContext* interpolated_string );

  std::unique_ptr<Value> string_value(
      EscriptGrammar::EscriptParser::StringLiteralContext* string_literal );

  std::string unquote( antlr4::tree::TerminalNode* regular_string );

  std::unique_ptr<Value> value( EscriptGrammar::EscriptParser::LiteralContext* );

private:
  int to_int( EscriptGrammar::EscriptParser::IntegerLiteralContext* );
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_VALUEBUILDER_H
