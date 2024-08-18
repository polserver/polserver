#pragma once

#include "bscript/compiler/astbuilder/CompoundStatementBuilder.h"

namespace Pol::Bscript::Compiler
{
class SuperFunction;

class GeneratedFunctionBuilder : public CompoundStatementBuilder
{
public:
  GeneratedFunctionBuilder( const SourceFileIdentifier&, BuilderWorkspace& );
  bool super_function( std::unique_ptr<SuperFunction>& );
};

}  // namespace Pol::Bscript::Compiler
