#include "Program.h"

#include <format/format.h>

#include "compiler/ast/FunctionBody.h"
#include "compiler/ast/NodeVisitor.h"
#include "compiler/ast/ProgramParameterList.h"

namespace Pol::Bscript::Compiler
{
Program::Program( const SourceLocation& source_location,
                  std::unique_ptr<ProgramParameterList> parameter_list,
                  std::unique_ptr<FunctionBody> body )
  : Node( source_location )
{
  children.reserve( 2 );
  children.push_back( std::move( parameter_list ) );
  children.push_back( std::move( body ) );
}

Program::~Program() = default;

void Program::accept( NodeVisitor& visitor )
{
  visitor.visit_program( *this );
}

void Program::describe_to( fmt::Writer& w ) const
{
  w << "program";
}

ProgramParameterList& Program::parameter_list()
{
  return child<ProgramParameterList>( 0 );
}

FunctionBody& Program::body()
{
  return child<FunctionBody>( 1 );
}

}  // namespace Pol::Bscript::Compiler
