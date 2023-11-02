/** @file
 *
 * @par History
 * - 2011/11/10 Tomi:    Added file
 * - 2011/12/13 Tomi:    Added rope, wheel, hull, sails, tiller, rudder, storage and weaponslot
 * objtypes for new boat support
 *                       Changed default values from 0xFXXX to 0x1FXXX
 */


#include "extobj.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "globals/settings.h"

namespace Pol
{
namespace Core
{
void read_extobj()
{
  Clib::ConfigFile cf;
  Clib::ConfigElem elem;

  if ( Clib::FileExists( "config/extobj.local.cfg" ) )
  {
    cf.open( "config/extobj.local.cfg" );
    cf.readraw( elem );
  }
  else if ( Clib::FileExists( "config/extobj.cfg" ) )
  {
    cf.open( "config/extobj.cfg" );
    cf.readraw( elem );
  }

  settingsManager.extobj.tillerman = elem.remove_unsigned( "Tillerman", 0x1F010 );
  settingsManager.extobj.port_plank = elem.remove_unsigned( "Port_Plank", 0x1F011 );
  settingsManager.extobj.starboard_plank = elem.remove_unsigned( "Starboard_Plank", 0x1F012 );
  settingsManager.extobj.hold = elem.remove_unsigned( "Hold", 0x1F013 );
  settingsManager.extobj.rope = elem.remove_unsigned( "Rope", 0x1F014 );
  settingsManager.extobj.wheel = elem.remove_unsigned( "Wheel", 0x1F015 );
  settingsManager.extobj.hull = elem.remove_unsigned( "Hull", 0x1F016 );
  settingsManager.extobj.tiller = elem.remove_unsigned( "Tiller", 0x1F017 );
  settingsManager.extobj.rudder = elem.remove_unsigned( "Rudder", 0x1F018 );
  settingsManager.extobj.sails = elem.remove_unsigned( "Sails", 0x1F019 );
  settingsManager.extobj.storage = elem.remove_unsigned( "Storage", 0x1F01A );
  settingsManager.extobj.weaponslot = elem.remove_unsigned( "Weaponslot", 0x1F01B );
  settingsManager.extobj.boatmount = elem.remove_unsigned( "Boatmount", 0x1F01C );

  settingsManager.extobj.wrestling = elem.remove_unsigned( "Wrestling", 0x1F020 );
  settingsManager.extobj.mount = elem.remove_unsigned( "Mount", 0x1F021 );
  settingsManager.extobj.shield = elem.remove_unsigned( "Shield", 0x1F022 );

  settingsManager.extobj.secure_trade_container =
      elem.remove_unsigned( "Secure_Trade_Container", 0x1FF01 );
  settingsManager.extobj.wornitems_container =
      elem.remove_unsigned( "Wornitems_Container", 0x1FF02 );
}
}
}
