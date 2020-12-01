#include "Variables.h"

#include "compiler/Report.h"
#include "compiler/file/SourceLocation.h"
#include "compiler/model/Variable.h"

namespace Pol::Bscript::Compiler
{
Variables::Variables( VariableScope scope, Report& report ) : scope( scope ), report( report ) {}

std::shared_ptr<Variable> Variables::create( const std::string& name, BlockDepth block_depth,
                                             WarnOn warn_on, const SourceLocation& source_location )
{
  auto index = names_by_index.size();
  if ( index > std::numeric_limits<VariableIndex>::max() ) {
    report.error(source_location, "Too many variables");
  }
  auto variable = std::make_shared<Variable>(
      scope, name, block_depth, static_cast<VariableIndex>( index ), warn_on, source_location );
  variables_by_name[name] = variable;
  names_by_index.push_back( name );
  return variable;
}

std::shared_ptr<Variable> Variables::find( const std::string& name ) const
{
  auto itr = variables_by_name.find( name );
  return ( itr != variables_by_name.end() ) ? ( *itr ).second : std::shared_ptr<Variable>();
}

void Variables::restore_shadowed( std::shared_ptr<Variable> variable )
{
  // It's still in names_by_index
  variables_by_name[variable->name] = std::move( variable );
}

void Variables::remove_all_but( unsigned count )
{
  while ( names_by_index.size() > count )
  {
    std::string last_name = names_by_index.back();
    auto itr = variables_by_name.find( last_name );

    if ( itr != variables_by_name.end() )
    {
      auto& removing = ( *itr ).second;
      if ( removing->warn_on == WarnOn::IfUsed && removing->was_used() )
      {
        report.warning( removing->source_location, "local variable '", last_name,
                        "' declared as unused but used.\n" );
      }
      else if ( removing->warn_on == WarnOn::IfNotUsed && !removing->was_used() )
      {
        report.warning( removing->source_location, "local variable '", last_name,
                        "' was not used.\n" );
      }
      variables_by_name.erase( itr );
    }
    names_by_index.pop_back();
  }
}

const std::vector<std::string>& Variables::get_names() const
{
  return names_by_index;
}

unsigned Variables::count() const
{
  return static_cast<unsigned>( names_by_index.size() );
}

}  // namespace Pol::Bscript::Compiler
