/** @file
 *
 * @par History
 */


#ifndef GUILDSMOD_H
#define GUILDSMOD_H

#include "../../bscript/execmodl.h"

namespace Pol
{
namespace Core
{
class Guild;
}
namespace Module
{
class GuildExecutorModule : public Bscript::TmplExecutorModule<GuildExecutorModule>
{
public:
  GuildExecutorModule( Bscript::Executor& exec );

  Bscript::BObjectImp* mf_ListGuilds();
  Bscript::BObjectImp* mf_CreateGuild();
  Bscript::BObjectImp* mf_FindGuild();
  Bscript::BObjectImp* mf_DestroyGuild();

  static Bscript::BObjectImp* CreateGuildRefObjImp( Core::Guild* guild );
};
}
}
#endif
