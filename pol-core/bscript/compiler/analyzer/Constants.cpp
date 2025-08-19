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
