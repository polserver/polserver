#include "Program.h"


#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/ProgramParameterList.h"

namespace Pol::Bscript::Compiler
{
Program::Program( const SourceLocation& source_location, const std::string& program_name,
                  std::unique_ptr<ProgramParameterList> parameter_list,
                  std::unique_ptr<FunctionBody> body )
    : Node( source_location ), program_name( program_name )
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

void Program::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "{}({})", type(), program_name );
}

std::string Program::type() const
{
  return "program";
}

void print(picojson::object&) {}
template <typename T1, typename T2, typename... Types>
void print( picojson::object& o, T1 var1, T2 var2, Types... var3 )
{
  o.insert( std::pair<T1, T2>( { var1, var2 } ) );
  print( o, var3... );
}

void Program::describe_to( picojson::object& o ) const
{
  print(o,
     "id", program_name
  );
  // o.insert( std::pair<std::string, std::string>( { "type", "program" } ) );
  // o.insert( std::pair<std::string, std::string>( { "id", program_name } ) );
};

ProgramParameterList& Program::parameter_list()
{
  return child<ProgramParameterList>( 0 );
}

FunctionBody& Program::body()
{
  return child<FunctionBody>( 1 );
}

}  // namespace Pol::Bscript::Compiler
