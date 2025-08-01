#include "Variables.h"

#include <ranges>

#include "bscript/compiler/Report.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/Variable.h"

namespace Pol::Bscript::Compiler
{
Variables::Variables( VariableScope scope, Report& report )
    : scope( scope ), report( report ), variable_info_stack( { VariablesInfo() } )
{
}

std::shared_ptr<Variable> Variables::create( const std::string& name, BlockDepth block_depth,
                                             WarnOn warn_on, const SourceLocation& source_location )
{
  auto index = current().names_by_index.size();
  if ( index > std::numeric_limits<VariableIndex>::max() )
  {
    report.error( source_location, "Too many variables" );
  }
  auto variable =
      std::make_shared<Variable>( scope, name, block_depth, static_cast<VariableIndex>( index ),
                                  warn_on, nullptr, source_location );
  current().variables_by_name[name] = variable;
  current().names_by_index.push_back( name );
  return variable;
}

std::shared_ptr<Variable> Variables::capture( std::shared_ptr<Variable>& other )
{
  // The new variable "local variable" representing the capture at runtime have
  // consecutive indexes.
  // At compilation:
  // - Keep track of the current executing function
  // - When visiting an identifier:
  // - If type == Capture: ValueStack offset will be current function's param count + this variable
  // index
  // - If type == Local: if offset > function param count, add current function's capture count
  auto index = static_cast<VariableIndex>( current().names_by_index.size() );
  auto captured =
      std::make_shared<Variable>( VariableScope::Capture, other->name, other->block_depth, index,
                                  other->warn_on, other, other->source_location );

  current().variables_by_name[other->name] = captured;
  current().names_by_index.push_back( captured->name );
  return captured;
}

std::shared_ptr<Variable> Variables::find( const std::string& name ) const
{
  auto itr = variable_info_stack.back().variables_by_name.find( name );
  return ( itr != variable_info_stack.back().variables_by_name.end() )
             ? ( *itr ).second
             : std::shared_ptr<Variable>();
}

std::shared_ptr<Variable> Variables::find_in_ancestors( const std::string& name ) const
{
  for ( const auto& info : variable_info_stack | std::views::reverse | std::views::drop( 1 ) )
  {
    auto itr = info.variables_by_name.find( name );

    if ( itr != info.variables_by_name.end() )
    {
      return ( *itr ).second;
    }
  }
  return nullptr;
}

void Variables::restore_shadowed( std::shared_ptr<Variable> variable )
{
  // It's still in names_by_index
  current().variables_by_name[variable->name] = std::move( variable );
}

void Variables::remove_all_but( unsigned count )
{
  while ( current().names_by_index.size() > count )
  {
    std::string last_name = current().names_by_index.back();
    auto itr = current().variables_by_name.find( last_name );

    if ( itr != current().variables_by_name.end() )
    {
      auto& removing = ( *itr ).second;
      if ( removing->warn_on == WarnOn::IfUsed && removing->was_used() )
      {
        report.warning( removing->source_location,
                        "local variable '{}' declared as unused but used.", last_name );
      }
      else if ( removing->warn_on == WarnOn::IfNotUsed && !removing->was_used() )
      {
        report.warning( removing->source_location, "local variable '{}' was not used.", last_name );
      }
      current().variables_by_name.erase( itr );
    }
    current().names_by_index.pop_back();
  }
}

const std::vector<std::string>& Variables::get_names() const
{
  return variable_info_stack.back().names_by_index;
}

unsigned Variables::count() const
{
  return static_cast<unsigned>( variable_info_stack.back().names_by_index.size() );
}

Variables::VariablesInfo& Variables::current()
{
  return variable_info_stack.back();
}

}  // namespace Pol::Bscript::Compiler
