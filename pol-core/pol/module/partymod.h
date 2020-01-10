/** @file
 *
 * @par History
 */


#ifndef PARTYMOD_H
#define PARTYMOD_H

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
class Party;
}
namespace Module
{
extern Bscript::BApplicObjType party_type;

class PartyExecutorModule : public Bscript::TmplExecutorModule<PartyExecutorModule, Core::PolModule>
{
public:
  PartyExecutorModule( Bscript::Executor& exec );

  Bscript::BObjectImp* mf_CreateParty();
  Bscript::BObjectImp* mf_DisbandParty();
  Bscript::BObjectImp* mf_SendPartyMsg();
  Bscript::BObjectImp* mf_SendPrivatePartyMsg();
};

Bscript::BObjectImp* CreatePartyRefObjImp( Core::Party* party );
}
}
#endif
