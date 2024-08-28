#pragma once

#include "bscript/compiler/astbuilder/CompoundStatementBuilder.h"

namespace Pol::Bscript::Compiler
{
class GeneratedFunction;
class ScopableName;
class UserFunction;

class GeneratedFunctionBuilder : public CompoundStatementBuilder
{
public:
  GeneratedFunctionBuilder( const SourceFileIdentifier&, BuilderWorkspace& );
  void super_function( std::unique_ptr<GeneratedFunction>& );
  void constructor_function( std::unique_ptr<GeneratedFunction>& );

private:
  void build( std::unique_ptr<GeneratedFunction>&, std::vector<UserFunction*> constructors );

  void add_base_constructor( std::unique_ptr<GeneratedFunction>&, UserFunction* constructor,
                             std::set<ScopableName>& visited_arg_names, bool can_use_rest );
};

}  // namespace Pol::Bscript::Compiler
