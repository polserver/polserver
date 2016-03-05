/** @file
 *
 * @par History
 */


#include "realm.h"

#include "../los.h"
#include "../mobile/charactr.h"

namespace Pol
{
namespace Realms
{
bool Realm::has_los( const Core::UObject& iatt, const Core::UObject& itgt ) const
{
  if ( iatt.realm != itgt.realm )
    return false;
  if ( iatt.isa( Core::UObject::CLASS_CHARACTER ) )
  {
    const Mobile::Character& chr = static_cast<const Mobile::Character&>( iatt );
    if ( chr.ignores_line_of_sight() )
      return true;
  }

  Core::LosObj att( iatt );
  Core::LosObj tgt( itgt );
  return has_los( att, tgt );
}

bool Realm::has_los( const Mobile::Character& iatt, const Core::UObject& itgt ) const
{
  bool remote;
  Items::Item* remote_container = iatt.search_remote_containers( itgt.serial, &remote );
  if ( ( remote_container != NULL ) && remote )
    return true;
  if ( iatt.realm != itgt.realm )
    return false;
  if ( iatt.ignores_line_of_sight() )
    return true;

  Core::LosObj att( iatt );
  Core::LosObj tgt( itgt );
  return has_los( att, tgt );
}

}
}