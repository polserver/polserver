#ifndef POLSERVER_VALUEBUILDER_H
#define POLSERVER_VALUEBUILDER_H

#include "compiler/astbuilder/TreeBuilder.h"

#include <EscriptGrammar/EscriptParser.h>

namespace Pol::Bscript::Compiler
{
class FloatValue;
class Value;

class ValueBuilder : public TreeBuilder
{
public:
  ValueBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  std::unique_ptr<FloatValue> float_value(
      EscriptGrammar::EscriptParser::FloatLiteralContext* );

  std::unique_ptr<Value> value( EscriptGrammar::EscriptParser::LiteralContext* );
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_VALUEBUILDER_H
