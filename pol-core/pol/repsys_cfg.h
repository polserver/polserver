/** @file
 *
 * @par History
 */


#ifndef REPSYS_CFG_H
#define REPSYS_CFG_H

namespace Pol
{
namespace Core
{
class ExportedFunction;

struct RepSys_Cfg
{
  struct
  {
    unsigned short Murderer;
    unsigned short Criminal;
    unsigned short Attackable;
    unsigned short Innocent;
    unsigned short GuildAlly;
    unsigned short GuildEnemy;
    unsigned short Invulnerable;
  } NameColoring;

  struct
  {
    unsigned short CriminalFlagInterval;
    unsigned short AggressorFlagTimeout;
    bool PartyHelpFullCountsAsCriminal;
    bool PartyHarmFullCountsAsCriminal;
  } General;

  struct
  {
    ExportedFunction* NameColor;
    ExportedFunction* HighLightColor;
    ExportedFunction* OnAttack;
    ExportedFunction* OnDamage;
    ExportedFunction* OnHelp;
  } Hooks;
};
}
}
#endif
