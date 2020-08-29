/** @file
 *
 * @par History
 */


#ifndef GUILDSMOD_H
#define GUILDSMOD_H

#include "../../bscript/bobject.h"
#include "../polmodl.h"

namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Core
{
class Guild;
}
namespace Module
{
extern Bscript::BApplicObjType guild_type;

class GuildExecutorModule : public Bscript::TmplExecutorModule<GuildExecutorModule, Core::PolModule>
{
public:
  GuildExecutorModule( Bscript::Executor& exec );

  [[nodiscard]] Bscript::BObjectImp* mf_ListGuilds();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateGuild();
  [[nodiscard]] Bscript::BObjectImp* mf_FindGuild();
  [[nodiscard]] Bscript::BObjectImp* mf_DestroyGuild();

  static Bscript::BObjectImp* CreateGuildRefObjImp( Core::Guild* guild );
};
}  // namespace Module
}  // namespace Pol
#endif
