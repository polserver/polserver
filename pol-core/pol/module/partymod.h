/** @file
 *
 * @par History
 */


#ifndef PARTYMOD_H
#define PARTYMOD_H

#include "../../bscript/bobject.h"
#include "../polmodl.h"


namespace Pol::Bscript
{
class Executor;
}  // namespace Pol::Bscript


namespace Pol
{
namespace Core
{
class Party;
}
namespace Module
{
class PartyExecutorModule : public Bscript::TmplExecutorModule<PartyExecutorModule, Core::PolModule>
{
public:
  PartyExecutorModule( Bscript::Executor& exec );

  [[nodiscard]] Bscript::BObjectImp* mf_CreateParty();
  [[nodiscard]] Bscript::BObjectImp* mf_DisbandParty();
  [[nodiscard]] Bscript::BObjectImp* mf_SendPartyMsg();
  [[nodiscard]] Bscript::BObjectImp* mf_SendPrivatePartyMsg();
};

Bscript::BObjectImp* CreatePartyRefObjImp( Core::Party* party );
}  // namespace Module
}  // namespace Pol
#endif
