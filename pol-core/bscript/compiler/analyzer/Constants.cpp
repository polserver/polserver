#include "Constants.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/ConstDeclaration.h"

namespace Pol::Bscript::Compiler
{
Constants::Constants( Report& report ) : report( report ) {}

ConstDeclaration* Constants::find( const std::string& name ) const
{
  auto itr = constants.find( name );
  return ( itr != constants.end() ) ? ( *itr ).second : nullptr;
}

std::vector<ConstDeclaration*> Constants::list( const std::string& prefix ) const
{
  std::vector<ConstDeclaration*> results;
  std::for_each( constants.begin(), constants.end(),
                 [&]( const auto& p )
                 {
                   if ( p.first.rfind( prefix, 0 ) == 0 )
                   {
                     results.push_back( p.second );
                   }
                 } );
  return results;
}


void Constants::create( ConstDeclaration& constant )
{
  auto itr = constants.find( constant.identifier );
  if ( itr != constants.end() )
  {
    if ( constant.ignore_overwrite_attempt )
    {
      // The OG compiler ignores enum members with the same name as a
      // constant or enum that was defined earlier.

      report.warning( constant, "Constant '", constant.identifier,
                      "' definition ignored due to an earlier definition.\n",
                      "  Previous definition at: ", ( *itr ).second->source_location );
      return;
    }
    report.error( constant, "Constant '", constant.identifier, "' defined more than once.\n",
                  "  Previous definition at: ", ( *itr ).second->source_location );
  }

  constants[constant.identifier] = &constant;
}

}  // namespace Pol::Bscript::Compiler
