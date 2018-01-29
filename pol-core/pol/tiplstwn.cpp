/** @file
 *
 * @par History
 */


#include <string>

#include "../clib/dirlist.h"
#include "globals/uvars.h"

namespace Pol
{
namespace Core
{
void load_tips()
{
  gamestate.tipfilenames.clear();

  for ( Clib::DirList dl( "tips/" ); !dl.at_end(); dl.next() )
  {
    std::string name = dl.name();
    if ( name[0] == '.' )
      continue;
    if ( name.find( ".txt" ) != std::string::npos )
    {
      gamestate.tipfilenames.push_back( name.c_str() );
    }
  }
}
}
}
