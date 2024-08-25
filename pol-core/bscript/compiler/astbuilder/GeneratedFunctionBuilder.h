#pragma once

#include "bscript/compiler/astbuilder/CompoundStatementBuilder.h"

namespace Pol::Bscript::Compiler
{
class GeneratedFunction;

class GeneratedFunctionBuilder : public CompoundStatementBuilder
{
public:
  GeneratedFunctionBuilder( const SourceFileIdentifier&, BuilderWorkspace& );
  void super_function( std::unique_ptr<GeneratedFunction>& );
  void constructor_function( std::unique_ptr<GeneratedFunction>& );
};

}  // namespace Pol::Bscript::Compiler
