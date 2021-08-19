#include "Constants.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/ConstDeclaration.h"

#include <algorithm>

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
  auto name = constant.name.string();
  auto itr = constants.find( name );
  if ( itr != constants.end() )
  {
    if ( constant.ignore_overwrite_attempt )
    {
      // The OG compiler ignores enum members with the same name as a
      // constant or enum that was defined earlier.

      report.warning( constant,
                      "Constant '{}' definition ignored due to an earlier definition.\n"
                      "  Previous definition at: {}",
                      name, ( *itr ).second->source_location );
      return;
    }
    report.error( constant,
                  "Constant '{}' defined more than once.\n"
                  "  Previous definition at: {}",
                  name, ( *itr ).second->source_location );
  }

  constants[name] = &constant;
}

}  // namespace Pol::Bscript::Compiler
