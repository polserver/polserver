#ifndef POLSERVER_USERFUNCTIONBUILDER_H
#define POLSERVER_USERFUNCTIONBUILDER_H

#include "CompoundStatementBuilder.h"

namespace Pol::Bscript::Compiler
{

class UserFunctionBuilder : public CompoundStatementBuilder
{
public:
  UserFunctionBuilder( const SourceFileIdentifier&, BuilderWorkspace& );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_USERFUNCTIONBUILDER_H
