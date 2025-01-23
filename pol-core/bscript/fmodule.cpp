/** @file
 *
 * @par History
 */

#include "fmodule.h"

#include <cstddef>

namespace Pol
{
namespace Bscript
{
ModuleFunction::ModuleFunction( const char* fname, int i_nargs )
    : name( fname ), nargs( i_nargs ), funcidx( -1 )
{
}

FunctionalityModule::FunctionalityModule( const char* i_modname )
    : have_indexes( false ), modulename( i_modname )
{
}

FunctionalityModule::~FunctionalityModule()
{
  while ( !functions.empty() )
  {
    delete functions.back();
    functions.pop_back();
  }
}

void FunctionalityModule::addFunction( const char* funcname, int nparams )
{
  auto mf = new ModuleFunction( funcname, nparams );
  mf->funcidx = static_cast<unsigned int>( functions.size() );
  functions.push_back( mf );
}
}  // namespace Bscript
}  // namespace Pol
