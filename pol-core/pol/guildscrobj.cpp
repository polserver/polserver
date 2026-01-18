#include "guildscrobj.h"

#include "../clib/stlutil.h"
#include "guilds.h"
#include <stddef.h>
#include <string>


namespace Pol::Module
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
    return false;
  }
  if ( objimp.isa( BObjectImp::OTBoolean ) )
    return isTrue() == static_cast<const BBoolean&>( objimp ).isTrue();
  return false;
}


}  // namespace Pol::Module
