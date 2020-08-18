#ifndef POLSERVER_VALUEBUILDER_H
#define POLSERVER_VALUEBUILDER_H

#include "compiler/astbuilder/TreeBuilder.h"

namespace Pol::Bscript::Compiler
{

class ValueBuilder : public TreeBuilder
{
public:
  ValueBuilder( const SourceFileIdentifier&, BuilderWorkspace& );
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_VALUEBUILDER_H
