/*
History
=======


Notes
=======

*/

#ifndef PARTYMOD_H
#define PARTYMOD_H

#include "../../bscript/execmodl.h"

class PartyExecutorModule : public TmplExecutorModule<PartyExecutorModule>
{
public:
	PartyExecutorModule( Executor& exec ) :
	TmplExecutorModule<PartyExecutorModule>( "Party", exec ) {};

	BObjectImp* mf_CreateParty();
	BObjectImp* mf_DisbandParty();
	BObjectImp* mf_SendPartyMsg();
	BObjectImp* mf_SendPrivatePartyMsg();
};

BObjectImp* CreatePartyRefObjImp( Party* party );

#endif