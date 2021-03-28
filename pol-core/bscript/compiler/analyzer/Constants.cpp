#include "Constants.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/ConstDeclaration.h"

namespace Pol::Bscript::Compiler
{
Constants::Constants( Report& report ) : report( report ) {}

ConstDeclaration* Constants::find( const std::string& name )
{
  auto itr = constants.find( name );
  return ( itr != constants.end() ) ? ( *itr ).second : nullptr;
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
