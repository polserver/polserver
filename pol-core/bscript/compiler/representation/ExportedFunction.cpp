#include "ExportedFunction.h"

namespace Pol::Bscript::Compiler
{
ExportedFunction::ExportedFunction( std::string name, unsigned parameter_count,
                                    unsigned entrypoint_program_counter )
  : name( std::move( name ) ),
    parameter_count( parameter_count ),
    entrypoint_program_counter( entrypoint_program_counter )
{
}

}  // namespace Pol::Bscript::Compiler
