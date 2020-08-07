#ifndef POLSERVER_EXPRESSIONBUILDER_H
#define POLSERVER_EXPRESSIONBUILDER_H

#include "ValueBuilder.h"

namespace Pol::Bscript::Compiler
{

class ExpressionBuilder : public ValueBuilder
{
public:
  ExpressionBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_EXPRESSIONBUILDER_H
