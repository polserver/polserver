/** @file
 *
 * @par History
 * - 2005/10/02 Shinigami: added Prop Script.attached_to and Script.controller
 * - 2005/11/25 Shinigami: MTH_GET_MEMBER/"get_member" - GCC fix
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2006/01/18 Shinigami: added attached_npc_ - to get attached NPC from AI-Script-Process Obj
 * - 2006/09/17 Shinigami: Script.sendevent() will return error "Event queue is full, discarding
 * event"
 * - 2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now
 * - 2009/12/21 Turley:    ._method() call fix
 */


#include "guildscrobj.h"

#include "guilds.h"
#include <stddef.h>
#include <string>
#include "../clib/stlutil.h"

namespace Pol
{
namespace Module
{
using namespace Bscript;

BApplicObjType guild_type;

EGuildRefObjImp::EGuildRefObjImp( Core::GuildRef gref )
    : PolApplicObj<Core::GuildRef>( &guild_type, gref ){};

const char* EGuildRefObjImp::typeOf() const
{
  return "GuildRef";
}
u8 EGuildRefObjImp::typeOfInt() const
{
  return OTGuildRef;
}

BObjectImp* EGuildRefObjImp::copy() const
{
  return new EGuildRefObjImp( obj_ );
}

bool EGuildRefObjImp::isTrue() const
{
  return ( !obj_->_disbanded );
}

bool EGuildRefObjImp::operator==( const BObjectImp& objimp ) const
{
  if ( objimp.isa( BObjectImp::OTApplicObj ) )
  {
    const BApplicObjBase* aob =
        Clib::explicit_cast<const BApplicObjBase*, const BObjectImp*>( &objimp );

    if ( aob->object_type() == &guild_type )
    {
      const EGuildRefObjImp* guildref_imp =
          Clib::explicit_cast<const EGuildRefObjImp*, const BApplicObjBase*>( aob );

      return ( guildref_imp->obj_->_guildid == obj_->_guildid );
    }
    else
      return false;
  }
  else if ( objimp.isa( BObjectImp::OTBoolean ) )
    return isTrue() == static_cast<const BBoolean&>( objimp ).isTrue();
  else
    return false;
}


}  // namespace Module
}  // namespace Pol
