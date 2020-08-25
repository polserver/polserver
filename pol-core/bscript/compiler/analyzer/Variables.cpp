#include "Variables.h"

#include "compiler/Report.h"
#include "compiler/file/SourceLocation.h"
#include "compiler/model/Variable.h"

namespace Pol::Bscript::Compiler
{
Variables::Variables( VariableScope scope, Report& report ) : scope( scope ), report( report ) {}

std::shared_ptr<Variable> Variables::create( const std::string& name, unsigned block_depth,
                                             WarnOn warn_on, const SourceLocation& source_location )
{
  auto index = names_by_index.size();
  auto variable =
      std::make_shared<Variable>( scope, name, block_depth, index, warn_on, source_location );
  variables_by_name[name] = variable;
  names_by_index.push_back( name );
  return variable;
}

std::shared_ptr<Variable> Variables::find( const std::string& name ) const
{
  auto itr = variables_by_name.find( name );
  return ( itr != variables_by_name.end() ) ? ( *itr ).second : std::shared_ptr<Variable>();
}

const std::vector<std::string>& Variables::get_names() const
{
  return names_by_index;
}

unsigned Variables::count() const
{
  return names_by_index.size();
}

}  // namespace Pol::Bscript::Compiler
