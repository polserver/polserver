#include "settings.h"

#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/fileutil.h"
#include "../../clib/logfacility.h"
#include "../../plib/systemstate.h"
#include "../watch.h"

namespace Pol
{
namespace Core
{
SettingsManager settingsManager;

SettingsManager::SettingsManager()
    : combat_config(),
      watch(),
      extobj(),
      party_cfg(),
      ssopt(),
      polvar(),
      movecost_walking(),
      movecost_running(),
      movecost_walking_mounted(),
      movecost_running_mounted(),
      repsys_cfg()
{
}

SettingsManager::~SettingsManager() {}


void SettingsManager::deinitialize() {}

size_t SettingsManager::estimateSize() const
{
  size_t size = sizeof( SettingsManager ) - sizeof( PolVar ) + polvar.estimateSize();
  return size;
}


void set_watch_vars()
{
  Clib::ConfigFile cf;
  Clib::ConfigElem elem;
  if ( Clib::FileExists( "config/watch.cfg" ) )
  {
    cf.open( "config/watch.cfg" );
    cf.readraw( elem );
  }
  else if ( Plib::systemstate.config.loglevel > 1 )
    INFO_PRINTLN( "File config/watch.cfg not found, skipping." );

  settingsManager.watch.combat = elem.remove_bool( "COMBAT", false );
  settingsManager.watch.profile_scripts = elem.remove_bool( "ProfileScripts", false );
}
}  // namespace Core
}  // namespace Pol
