#ifndef POLSERVER_VALUEBUILDER_H
#define POLSERVER_VALUEBUILDER_H

#include "compiler/astbuilder/TreeBuilder.h"

#include <EscriptGrammar/EscriptParser.h>

namespace Pol::Bscript::Compiler
{
class IntegerValue;
class FloatValue;
class StringValue;
class Value;

class ValueBuilder : public TreeBuilder
{
public:
  ValueBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  std::unique_ptr<FloatValue> float_value(
      EscriptGrammar::EscriptParser::FloatLiteralContext* );

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
