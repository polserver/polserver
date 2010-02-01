/*
History
=======
2009/12/21 Turley:    ._method() call fix

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef MEMORYLEAK
#include "../../bscript/bobject.h"
#endif
#include "../../bscript/berror.h"
#include "../../bscript/objmembers.h"
#include "../../bscript/objmethods.h"
#include "../../bscript/bobject.h"
#include "../../clib/unicode.h"
#include "../../clib/stlutil.h"
#include "../../clib/cfgelem.h"
#include "../network/client.h"
#include "../fnsearch.h"
#include "../proplist.h"
#include "../party.h"
#include "../clfunc.h"
#include "../uoscrobj.h"
#include "../uoexhelp.h"
#include "partymod.h"

template<>
TmplExecutorModule<PartyExecutorModule>::FunctionDef
TmplExecutorModule<PartyExecutorModule>::function_table[] =
{
	{ "CreateParty",			&PartyExecutorModule::mf_CreateParty },
	{ "DisbandParty",			&PartyExecutorModule::mf_DisbandParty },
	{ "SendPartyMsg",			&PartyExecutorModule::mf_SendPartyMsg },
	{ "SendPrivatePartyMsg",	&PartyExecutorModule::mf_SendPrivatePartyMsg },
};
template<>
int TmplExecutorModule<PartyExecutorModule>::function_table_size =
arsize(function_table);

class EPartyRefObjImp : public BApplicObj<PartyRef>
{
public:
	EPartyRefObjImp( PartyRef pref );
	virtual const char* typeOf() const;
	virtual BObjectImp* copy() const;
	virtual bool isTrue() const;
	virtual bool isEqual(const BObjectImp& objimp) const;

	virtual BObjectRef get_member( const char* membername );
	virtual BObjectRef get_member_id( const int id ); //id test
	virtual BObjectImp* call_method( const char* methodname, Executor& ex );
	virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
};
BApplicObjType party_type;
EPartyRefObjImp::EPartyRefObjImp( PartyRef pref ) : BApplicObj<PartyRef>(&party_type,pref)
{
};

const char* EPartyRefObjImp::typeOf() const
{
	return "PartyRef";
}

BObjectImp* EPartyRefObjImp::copy() const
{
	return new EPartyRefObjImp( obj_ );
}

bool EPartyRefObjImp::isTrue() const
{
	return (obj_->test_size());   
}

bool EPartyRefObjImp::isEqual(const BObjectImp& objimp) const
{
	if (objimp.isa( BObjectImp::OTApplicObj ))
	{
		const BApplicObjBase* aob = explicit_cast<const BApplicObjBase*, const BObjectImp*>(&objimp);

		if (aob->object_type() == &party_type)
		{
			const EPartyRefObjImp* partyref_imp =
				explicit_cast<const EPartyRefObjImp*,const BApplicObjBase*>(aob);

			return (partyref_imp->obj_->leader() == obj_->leader());
		}
		else
			return false;
	}
	else
		return false;
}

BObjectImp* CreatePartyRefObjImp( Party* party )
{
	return new EPartyRefObjImp( ref_ptr<Party>(party) );
}

BObjectRef EPartyRefObjImp::get_member( const char* membername )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if( objmember != NULL )
		return this->get_member_id(objmember->id);
	else
		return BObjectRef(UninitObject::create());
}

BObjectRef EPartyRefObjImp::get_member_id( const int id ) //id test
{
	switch(id)
	{
	case MBR_MEMBERS:
		{
			ObjArray* arr = new ObjArray;
			vector<u32>::iterator itr = obj_->_member_serials.begin();
			while( itr != obj_->_member_serials.end() )
			{
				Character* chr = system_find_mobile( *itr );
				if (chr != NULL)
				{
					arr->addElement( new EOfflineCharacterRefObjImp( chr ) );
					++itr;
				}
				else
					itr = obj_->_member_serials.erase( itr );
			}
			return BObjectRef( arr );
		}

	case MBR_LEADER:
		{
			Character* chr = system_find_mobile( obj_->_leaderserial );
			if (chr!=NULL)
				return BObjectRef(new EOfflineCharacterRefObjImp(chr));
			else
				return BObjectRef(new BLong(0));
		}

	case MBR_CANDIDATES:
		{
			ObjArray* arr = new ObjArray;
			vector<u32>::iterator itr = obj_->_candidates_serials.begin();
			while( itr != obj_->_candidates_serials.end() )
			{
				Character* chr = system_find_mobile( *itr );
				if (chr != NULL)
				{
					arr->addElement( new EOfflineCharacterRefObjImp( chr ) );
					++itr;
				}
				else
					itr = obj_->_candidates_serials.erase( itr );
			}
			return BObjectRef( arr );
		}

	default:
		return BObjectRef( UninitObject::create() );
	}
}

BObjectImp* EPartyRefObjImp::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
	{
		bool changed = false;
		return CallPropertyListMethod( obj_->_proplist, methodname, ex, changed );
	}
}

BObjectImp* EPartyRefObjImp::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
	switch(id)
	{
	case MTH_ADDMEMBER:
		{
			Character* chr;
			if (!ex.hasParams( 1 ))
				return new BError( "Not enough parameters" );
			if (!getCharacterParam( ex, 0, chr ))
				return new BError( "Invalid parameter type" );
			if (chr->party() != NULL)
				return new BError("Character is already in a party");
			else if (chr->candidate_of() != NULL)
				return new BError("Character is already candidate of a party");
			else if (chr->offline_mem_of() != NULL)
				return new BError("Character is already offline member of a party");
			if (!obj_->can_add())
				return new BError("Party is already full");
			if (obj_->add_member(chr->serial))
			{
				chr->party(obj_.get());
				if (party_cfg.Hooks.OnAddToParty)
					party_cfg.Hooks.OnAddToParty->call( chr->make_ref() );
				if (chr->has_active_client())
					send_sysmessage_cl(chr->client, CLP_Added);// You have been added to the party.
				obj_->send_msg_to_all(CLP_Joined, chr->name().c_str(), chr);//  : joined the party.
				obj_->send_member_list(NULL);
				obj_->send_stats_on_add(chr);
			}

			return new BLong(obj_->_member_serials.size());
		}

	case MTH_REMOVEMEMBER:
		{
			Character* chr;
			if (!ex.hasParams( 1 ))
				return new BError( "Not enough parameters" );
			if (!getCharacterParam( ex, 0, chr ))
				return new BError( "Invalid parameter type" );
			if (!obj_->is_member(chr->serial))
				return new BError("Character is not in this party");
			if (obj_->is_leader(chr->serial))
				return new BError("Character is leader of this party");

			if (obj_->remove_member(chr->serial))
			{
				bool disband;
				obj_->send_remove_member(chr,&disband);
				chr->party(NULL);
				if (party_cfg.Hooks.OnLeaveParty)
					party_cfg.Hooks.OnLeaveParty->call( chr->make_ref(), new BLong(0));
				if (chr->has_active_client())
				{
					send_sysmessage_cl(chr->client, CLP_Removed); //You have been removed from the party.
					send_empty_party(chr);
				}
				if (disband)
					disband_party(obj_->leader());

				return new BLong(1);
			}
			return new BLong(0);
		}

	case MTH_SETLEADER:
		{
			Character* chr;
			if (!ex.hasParams( 1 ))
				return new BError( "Not enough parameters" );
			if (!getCharacterParam( ex, 0, chr ))
				return new BError( "Invalid parameter type" );
			if (!obj_->is_member(chr->serial))
				return new BError("Character is not in this party");
			if (obj_->is_leader(chr->serial))
				return new BError("Character is already leader of this party");

			obj_->set_leader(chr->serial);
			obj_->send_member_list(NULL);
			return new BLong(1);
		}

	case MTH_ADDCANDIDATE:
		{
			Character* chr;
			if (!ex.hasParams( 1 ))
				return new BError( "Not enough parameters" );
			if (!getCharacterParam( ex, 0, chr ))
				return new BError( "Invalid parameter type" );
			if (chr->party()!=NULL)
				return new BError("Character is already in a party");
			else if (chr->candidate_of()!=NULL)
				return new BError("Character is already candidate of a party");
			else if (chr->offline_mem_of() != NULL)
				return new BError("Character is already offline member of a party");
			if (!obj_->can_add())
				return new BError("Party is already full");
			if (!chr->has_active_client())
				return new BError("Character is offline");

			if (party_cfg.General.DeclineTimeout > 0 )
				chr->set_party_invite_timeout();

			if (obj_->add_candidate(chr->serial))
			{
				Character* leader = system_find_mobile(obj_->leader());
				if (leader!=NULL)
				{
					chr->candidate_of(obj_.get());
					send_invite(chr,leader);
					return new BLong(1);
				}
			}
			return new BLong(0);
		}

	case MTH_REMOVECANDIDATE:
		{
			Character* chr;
			if (!ex.hasParams( 1 ))
				return new BError( "Not enough parameters" );
			if (!getCharacterParam( ex, 0, chr ))
				return new BError( "Invalid parameter type" );
			if (!obj_->is_candidate(chr->serial))
				return new BError("Character is not candidate of this party");

			if (obj_->remove_candidate(chr->serial))
			{
				chr->cancel_party_invite_timeout();
				Character* leader = system_find_mobile(obj_->leader());
				chr->candidate_of(NULL);
				if (chr->has_active_client())
					send_sysmessage_cl(chr->client, CLP_Decline); // You notify them that you do not wish to join the party.
				if (leader!=NULL)
				{
					if (leader->has_active_client())
						send_sysmessage_cl_affix(leader->client, CLP_Notify_Decline, chr->name().c_str(),true); //: Does not wish to join the party.
				}

				if (!obj_->test_size())
					disband_party(obj_->leader());
				return new BLong(1);
			}
			return new BLong(0);
		}

	default:
		bool changed = false;
		return CallPropertyListMethod_id( obj_->_proplist, id, ex, changed );
	}
}

// party.em Functions:
bool getPartyParam( Executor& exec, 
				   unsigned param,
				   Party*& party,
				   BError*& err )
{
	BApplicObjBase* aob = NULL;
	if (exec.hasParams( param+1 ))
		aob = exec.getApplicObjParam( param, &party_type );

	if (aob == NULL)
	{
		err = new BError( "Invalid parameter type" );
		return false;
	}

	EPartyRefObjImp* pr = static_cast<EPartyRefObjImp*>(aob);
	party = pr->value().get();
	if (system_find_mobile(party->leader())==NULL)
	{
		err = new BError( "Party has no leader" );
		return false;
	}
	return true;
}

BObjectImp* PartyExecutorModule::mf_CreateParty()
{
	Character* leader;
	Character* firstmem;
	if ((getCharacterParam( exec, 0, leader )) &&
		(getCharacterParam( exec, 1, firstmem )))
	{
		if (leader->party()!=NULL)
			return new BError("Leader is already in a party");
		else if (leader->candidate_of()!=NULL)
			return new BError("Leader is already candidate of a party");
		else if (leader->offline_mem_of()!=NULL)
			return new BError("Leader is already offline member of a party");
		else if (leader==firstmem)
			return new BError("Leader and Firstmember are the same");
		else if (firstmem->party()!=NULL)
			return new BError("First Member is already in a party");
		else if (firstmem->candidate_of()!=NULL)
			return new BError("First Member is already candidate of a party");
		else if (firstmem->offline_mem_of()!=NULL)
			return new BError("First Member is already offline member of a party");

		Party* party = new Party(leader->serial);
		parties.push_back(ref_ptr<Party>(party));
		leader->party(party);

		if (party->add_member(firstmem->serial))
		{
			firstmem->party(party);
			if (party_cfg.Hooks.OnPartyCreate)
				party_cfg.Hooks.OnPartyCreate->call( CreatePartyRefObjImp( party ) );
			party->send_msg_to_all(CLP_Added);// You have been added to the party.
			if (leader->has_active_client())
				send_sysmessage_cl_affix(leader->client,CLP_Joined,firstmem->name().c_str(),true);//  : joined the party.
			if (firstmem->has_active_client())
				send_sysmessage_cl_affix(firstmem->client,CLP_Joined,leader->name().c_str(),true);
			party->send_member_list(NULL);
			party->send_stats_on_add(firstmem);
		}

		return new BLong(1);
	}
	else
		return new BError( "Invalid parameter type" );
}

BObjectImp* PartyExecutorModule::mf_DisbandParty()
{
	Party* party;
	BError* err;
	if (getPartyParam( exec, 0, party, err ))
	{
		disband_party(party->leader());
		return new BLong(1);
	}
	else
		return err;
}

BObjectImp* PartyExecutorModule::mf_SendPartyMsg()
{
	Party* party;
	BError* err;
	ObjArray* oText;
	Character* chr;
	if (getPartyParam( exec, 0, party, err ))
	{
		if ((getCharacterParam( exec, 1, chr )) &&
			(getObjArrayParam( 2, oText )))
		{
			unsigned textlen = oText->ref_arr.size();
			u16 gwtext[ (SPEECH_MAX_LEN + 1) ];
			if ( textlen > SPEECH_MAX_LEN )
				return new BError( "Unicode array exceeds maximum size." );
			if ( !convertArrayToUC(oText, gwtext, textlen, true) )
				return new BError( "Invalid value in Unicode array." );

			if (party_cfg.Hooks.OnPublicChat)
				party_cfg.Hooks.OnPublicChat->call(chr->make_ref(), oText);

			party->send_member_msg_public(chr,gwtext,textlen);
			return new BLong(1);
		}
		else
			return new BError( "Invalid parameter type" );
	}
	else
		return err;
}

BObjectImp* PartyExecutorModule::mf_SendPrivatePartyMsg()
{
	Party* party;
	BError* err;
	ObjArray* oText;
	Character* chr;
	Character* tochr;
	if (getPartyParam( exec, 0, party, err ))
	{
		if ((getCharacterParam( exec, 1, chr )) &&
			(getCharacterParam( exec, 2, tochr )) &&
			(getObjArrayParam( 3, oText )))
		{
			unsigned textlen = oText->ref_arr.size();
			u16 gwtext[ (SPEECH_MAX_LEN + 1) ];
			if ( textlen > SPEECH_MAX_LEN )
				return new BError( "Unicode array exceeds maximum size." );
			if ( !convertArrayToUC(oText, gwtext, textlen, true) )
				return new BError( "Invalid value in Unicode array." );

			if (party_cfg.Hooks.OnPrivateChat)
				party_cfg.Hooks.OnPrivateChat->call(chr->make_ref(),tochr->make_ref(), oText);

			party->send_member_msg_private(chr,tochr,gwtext,textlen);
			return new BLong(1);
		}
		else
			return new BError( "Invalid parameter type" );
	}
	else
		return err;
}

