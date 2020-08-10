#ifndef POLSERVER_PROGRAMBUILDER_H
#define POLSERVER_PROGRAMBUILDER_H

#include "compiler/astbuilder/CompoundStatementBuilder.h"

namespace Pol::Bscript::Compiler
{

class ProgramBuilder : public CompoundStatementBuilder
{
public:
  ProgramBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

};


}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_PROGRAMBUILDER_H
