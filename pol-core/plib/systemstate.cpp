
#include "systemstate.h"

#include "../clib/stlutil.h"
#include "pkg.h"
#include "pol/objtype.h"
#include "tiles.h"

namespace Pol
{
namespace Plib
{
SystemState systemstate;


SystemState::SystemState()
    : packages(),
      packages_byname(),
      accounts_txt_dirty( false ),
      accounts_txt_stat(),
      config(),
      tile( UOBJ_HSA_MAX + 1 ),
      max_graphic( 0 )
{
}

void SystemState::deinitialize()
{
  Clib::delete_all( packages );
  packages_byname.clear();
  tile.clear();
}

size_t SystemState::estimatedSize() const
{
  size_t size = sizeof( SystemState );
  size += ( config.max_tile_id + 1 ) * sizeof( Tile );

  size += 3 * sizeof( Package** ) + packages.capacity() * sizeof( Package* );
  for ( const auto& pkg : packages )
    if ( pkg != nullptr )
      size += pkg->estimateSize();

  for ( const auto& pkg_pair : packages_byname )
    size += pkg_pair.first.capacity() + sizeof( Package* ) + ( sizeof( void* ) * 3 + 1 ) / 2;
  return size;
}
}  // namespace Plib
}  // namespace Pol
