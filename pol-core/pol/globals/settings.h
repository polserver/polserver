#ifndef GLOBALS_SETTINGS_H
#define GLOBALS_SETTINGS_H

#include "pol/cmbtcfg.h"
#include "pol/email_cfg.h"
#include "pol/extobj.h"
#include "pol/movecost.h"
#include "pol/party_cfg.h"
#include "pol/polvar.h"
#include "pol/repsys_cfg.h"
#include "pol/ssopt.h"
#include "pol/watch.h"


namespace Pol::Core
{
class SettingsManager
{
public:
  SettingsManager();
  ~SettingsManager();
  SettingsManager( const SettingsManager& ) = delete;
  SettingsManager& operator=( const SettingsManager& ) = delete;

  void deinitialize();
  [[nodiscard]] size_t estimateSize() const;

  CombatConfig combat_config;
  Watch watch;
  ExternalObject extobj;
  Party_Cfg party_cfg;
  ServSpecOpt ssopt;
  PolVar polvar;

  MovementCost movecost_walking;
  MovementCost movecost_running;
  MovementCost movecost_walking_mounted;
  MovementCost movecost_running_mounted;

  RepSys_Cfg repsys_cfg;
  Email_Cfg email_cfg;
};

extern SettingsManager settingsManager;
}  // namespace Pol::Core

#endif
