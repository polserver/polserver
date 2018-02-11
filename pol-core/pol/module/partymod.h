/** @file
 *
 * @par History
 */


#ifndef PARTYMOD_H
#define PARTYMOD_H

#include "../../bscript/execmodl.h"

namespace Pol
{
namespace Core
{
class Party;
}
namespace Module
{
class PartyExecutorModule : public Bscript::TmplExecutorModule<PartyExecutorModule>
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
