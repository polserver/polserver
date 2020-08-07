#ifndef POLSERVER_VALUEBUILDER_H
#define POLSERVER_VALUEBUILDER_H

#include "TreeBuilder.h"
#include "EscriptGrammar/EscriptParser.h"

namespace Pol::Bscript::Compiler
{
class FloatValue;
class FunctionReference;
class IntegerValue;
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

  std::unique_ptr<StringValue> string_value( antlr4::tree::TerminalNode* string_literal );

  std::string unquote( antlr4::tree::TerminalNode* string_literal );

  std::unique_ptr<Value> value( EscriptGrammar::EscriptParser::LiteralContext* );

private:
  int to_int( EscriptGrammar::EscriptParser::IntegerLiteralContext* );
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_VALUEBUILDER_H
