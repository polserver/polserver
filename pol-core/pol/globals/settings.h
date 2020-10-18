#ifndef GLOBALS_SETTINGS_H
#define GLOBALS_SETTINGS_H

#include "../cmbtcfg.h"
#include "../extobj.h"
#include "../movecost.h"
#include "../party_cfg.h"
#include "../polvar.h"
#include "../repsys_cfg.h"
#include "../ssopt.h"
#include "../watch.h"

namespace Pol
{
namespace Core
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
};

extern SettingsManager settingsManager;
}  // namespace Core
}  // namespace Pol
#endif
