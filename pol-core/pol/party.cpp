/*
History
=======

2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of _leaderserial
                      STLport-5.2.1 fix: UCconv, bytemsg and wtextoffset not used
					  STLport-5.2.1 fix: "wtext[ SPEECH_MAX_LEN+1 ];" has no effect
					  STLport-5.2.1 fix: illegal usage of NULL instead of 0
					  STLport-5.2.1 fix: initialization of member in handle_party_msg()
2009/08/26 Turley:    changed convertUCtoArray() to if check

Notes
=======

*/

#include "clib/stl_inc.h"

#include "bscript/berror.h"
#ifdef MEMORYLEAK
#include "bscript/bobject.h"
#endif
#include "clib/endian.h"


#include "plib/realm.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "fnsearch.h"
#include "polcfg.h"
#include "statmsg.h"
#include "syshook.h"
#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/cfgsect.h"
#include "clib/fileutil.h"
#include "clib/refptr.h"
#include "party.h"
#include "pktboth.h"
#include "clfunc.h"
#include "target.h"
#include "uobject.h"
#include "module/uomod.h"
#include "uoscrobj.h"
#include "bscript/objmembers.h"
#include "bscript/objmethods.h"
#include "bscript/bobject.h"
#include "clib/unicode.h"
#include "bscript/impstr.h"
#include "schedule.h"
#include "clib/stlutil.h"

const int CLP_Rejoined           = 1005437; // You have rejoined the party.
const int CLP_Add_Yourself       = 1005439; // You cannot add yourself to a party.
const int CLP_Already_Your_Party = 1005440; // This person is already in your party!
const int CLP_Already_in_a_Party = 1005441; // This person is already in a party!
const int CLP_Add_Living         = 1005442; // You may only add living things to your party!
const int CLP_Ignore_Offer       = 1005444; // The creature ignores your offer.
const int CLP_Added              = 1005445; // You have been added to the party.
const int CLP_Cannot_Remove_Self = 1005446; // You may only remove yourself from a party if you are not the leader.
const int CLP_Allow_Loot         = 1005447; // You have chosen to allow your party to loot your corpse.
const int CLP_Prevent_Loot       = 1005448; // You have chosen to prevent your party from looting your corpse.
const int CLP_Disbanded          = 1005449; // Your party has disbanded.
const int CLP_Last_Person        = 1005450; // The last person has left the party...
const int CLP_Removed            = 1005451; // You have been removed from the party.
const int CLP_Player_Removed     = 1005452; // A player has been removed from your party.
const int CLP_Add_No_Leader      = 1005453; // You may only add members to the party if you are the leader.
const int CLP_Who_To_Add         = 1005454; // Who would you like to add to your party?
const int CLP_Who_to_Remove      = 1005455; // Who would you like to remove from your party?
const int CLP_Player_Rejoined    = 1008087; // : rejoined the party.
const int CLP_Invite             = 1008089; // : You are invited to join the party. Type /accept to join or /decline to decline the offer.
const int CLP_Invited            = 1008090; // You have invited them to join the party.
const int CLP_Notify_Decline     = 1008091; // : Does not wish to join the party.
const int CLP_Decline            = 1008092; // You notify them that you do not wish to join the party.
const int CLP_Joined             = 1008094; // : joined the party.
const int CLP_Max_Size           = 1008095; // You may only have 10 in your party (this includes candidates).
const int CLP_No_Party           = 3000211; // You are not in a party.
const int CLP_No_Invite          = 3000222; // No one has invited you to be in a party.
const int CLP_Too_Late_Decline   = 3000223; // Too late to decline, you are already in the party. Try /quit.

struct Party_Cfg {
	struct {
		unsigned short MaxPartyMembers;
		bool TreatNoAsPrivate;
		unsigned short DeclineTimeout;
		u16 PrivateMsgPrefix[SPEECH_MAX_LEN+1 ];
		unsigned int PrivateMsgPrefixLen;
		bool RemoveMemberOnLogoff;
		bool RejoinPartyOnLogon;
	} General;

	struct {
		ExportedFunction* CanAddToParty;     // (leader,member)
		ExportedFunction* CanLeaveParty;     // (member
		ExportedFunction* CanRemoveMember;   // (leader,member)
		ExportedFunction* ChangePrivateChat; // (member,tomember,uctext)
		ExportedFunction* ChangePublicChat;  // (member,uctext)
		ExportedFunction* OnAddToParty;      // (who)
		ExportedFunction* OnDecline;         // (who)
		ExportedFunction* OnDisband;         // (partyref)
		ExportedFunction* OnLeaveParty;      // (who,fromwho)
		ExportedFunction* OnLootPermChange;  // (who)
		ExportedFunction* OnPartyCreate;     // (party)
		ExportedFunction* OnPrivateChat;     // (member,tomember,uctext)
		ExportedFunction* OnPublicChat;      // (member,uctext)
	} Hooks;

} party_cfg;

typedef ref_ptr<Party> PartyRef;

typedef vector <PartyRef> Parties;
static Parties parties;

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
    virtual BObjectImp* call_method_id( const int id, Executor& ex );
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
    ObjArray* arr;
	Character* chr;
	vector<u32>::iterator itr;
    switch(id)
    {
    case MBR_MEMBERS:
        arr = new ObjArray;
		itr = obj_->_member_serials.begin();
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

	case MBR_LEADER:
		chr = system_find_mobile( obj_->_leaderserial );
		if (chr!=NULL)
			return BObjectRef(new EOfflineCharacterRefObjImp(chr));
		else
			return BObjectRef(new BLong(0));

	case MBR_CANDIDATES:
		arr = new ObjArray;
		itr = obj_->_candidates_serials.begin();
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

BObjectImp* EPartyRefObjImp::call_method_id( const int id, Executor& ex )
{
	Character* chr;
	Character* leader;
	
    switch(id)
    {
    case MTH_ADDMEMBER:
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
		
	case MTH_REMOVEMEMBER:
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

	case MTH_SETLEADER:
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

	case MTH_ADDCANDIDATE:
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
			leader = system_find_mobile(obj_->leader());
			if (leader!=NULL)
			{
				chr->candidate_of(obj_.get());
				send_invite(chr,leader);
				return new BLong(1);
			}
		}
		return new BLong(0);

	case MTH_REMOVECANDIDATE:
		if (!ex.hasParams( 1 ))
            return new BError( "Not enough parameters" );
        if (!getCharacterParam( ex, 0, chr ))
            return new BError( "Invalid parameter type" );
		if (!obj_->is_candidate(chr->serial))
			return new BError("Character is not candidate of this party");

		if (obj_->remove_candidate(chr->serial))
		{
			chr->cancel_party_invite_timeout();
			leader = system_find_mobile(obj_->leader());
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

	default:
        bool changed = false;
        return CallPropertyListMethod_id( obj_->_proplist, id, ex, changed );
	}
}

Party::Party( u32 serial ) :
  _member_serials(),
  _candidates_serials(),
  _offlinemember_serials(),
  _leaderserial(serial)
{
	_member_serials.push_back(serial);
}

Party::Party( ConfigElem& elem ) :
  _member_serials(),
  _candidates_serials(),
  _offlinemember_serials(),
  _leaderserial(0)
{
	_leaderserial=elem.remove_ulong("LEADER");
	unsigned long tmp;
    while (elem.remove_prop( "MEMBER", &tmp ))
    {
		_member_serials.push_back(tmp);
    }
	_proplist.readProperties( elem );
}

void load_party_cfg_general( ConfigElem& elem )
{
	party_cfg.General.MaxPartyMembers = elem.remove_ushort( "MaxPartyMembers", 10 );
	party_cfg.General.TreatNoAsPrivate = elem.remove_bool( "TreatNoAsPrivate", false );
	party_cfg.General.DeclineTimeout = elem.remove_ushort( "DeclineTimeout", 10 );
	party_cfg.General.RemoveMemberOnLogoff = elem.remove_bool( "RemoveMemberOnLogoff",false);
	if (party_cfg.General.RemoveMemberOnLogoff)
		party_cfg.General.RejoinPartyOnLogon = elem.remove_bool("RejoinPartyOnLogon",false);
	else
		party_cfg.General.RejoinPartyOnLogon = false;
	string tmp = elem.remove_string("PrivateMsgPrefix","");
	if (tmp.size()==0)
		party_cfg.General.PrivateMsgPrefixLen=0;
	else
	{
		ObjArray* arr = new ObjArray;
		tmp+=" ";
		for( unsigned i = 0; i < tmp.size(); ++i )
		{
			arr->addElement( new BLong( static_cast<unsigned char>(tmp[i]) ) );
		}
		party_cfg.General.PrivateMsgPrefixLen = arr->ref_arr.size();
		if ( !convertArrayToUC(arr, party_cfg.General.PrivateMsgPrefix, party_cfg.General.PrivateMsgPrefixLen, true) )
			party_cfg.General.PrivateMsgPrefixLen=0;
		delete arr;
	}
}

void load_party_cfg_hooks( ConfigElem& elem )
{
	string temp;
	if ( elem.remove_prop("CanAddToParty", &temp) )
		party_cfg.Hooks.CanAddToParty = FindExportedFunction(elem, NULL, temp, 2);
	if ( elem.remove_prop("CanRemoveMember", &temp) )
		party_cfg.Hooks.CanRemoveMember = FindExportedFunction(elem, NULL, temp, 2);
	if ( elem.remove_prop("CanLeaveParty", &temp) )
		party_cfg.Hooks.CanLeaveParty = FindExportedFunction(elem, NULL, temp, 1);
	if ( elem.remove_prop("OnPublicChat", &temp) )
		party_cfg.Hooks.OnPublicChat = FindExportedFunction(elem, NULL, temp, 2);
	if ( elem.remove_prop("OnPrivateChat", &temp) )
		party_cfg.Hooks.OnPrivateChat = FindExportedFunction(elem, NULL, temp, 3);
	if ( elem.remove_prop("OnDisband", &temp) )
		party_cfg.Hooks.OnDisband = FindExportedFunction(elem, NULL, temp, 1);
	if ( elem.remove_prop("ChangePublicChat", &temp) )
		party_cfg.Hooks.ChangePublicChat = FindExportedFunction(elem, NULL, temp, 2);
	if ( elem.remove_prop("ChangePrivateChat", &temp) )
		party_cfg.Hooks.ChangePrivateChat = FindExportedFunction(elem, NULL, temp, 3);
	if ( elem.remove_prop("OnLeaveParty", &temp) )
		party_cfg.Hooks.OnLeaveParty = FindExportedFunction(elem, NULL, temp, 2);
	if ( elem.remove_prop("OnAddToParty", &temp) )
		party_cfg.Hooks.OnAddToParty = FindExportedFunction(elem, NULL, temp, 1);
	if ( elem.remove_prop("OnPartyCreate", &temp) )
		party_cfg.Hooks.OnPartyCreate = FindExportedFunction(elem, NULL, temp, 1);
	if ( elem.remove_prop("OnDecline", &temp) )
		party_cfg.Hooks.OnDecline = FindExportedFunction(elem, NULL, temp, 1);
	if ( elem.remove_prop("OnLootPermChange", &temp) )
		party_cfg.Hooks.OnLootPermChange = FindExportedFunction(elem, NULL, temp, 1);
}
void load_party_cfg( bool reload )
{
	if(!FileExists("config/party.cfg"))
	{
		party_cfg.General.MaxPartyMembers = 10;
		party_cfg.General.TreatNoAsPrivate = false;
		party_cfg.General.DeclineTimeout = 10;
		party_cfg.General.PrivateMsgPrefixLen = 0;
		party_cfg.General.RemoveMemberOnLogoff = false;
		party_cfg.General.RejoinPartyOnLogon = false;
	}
	else
	{
		ConfigFile cf("config/party.cfg");
		
		ConfigSection general(cf, "General");
		ConfigSection hooks(cf, "HookList", CST_NORMAL);

		ConfigElem elem;

		while ( cf.read( elem ) )
		{
			if ( general.matches(elem) )
				load_party_cfg_general(elem);
			else if ( hooks.matches(elem) && !reload )
				load_party_cfg_hooks(elem);
		}
	}
}

void unload_party_hooks()
{
	if (party_cfg.Hooks.CanAddToParty != NULL) {
		delete party_cfg.Hooks.CanAddToParty;
		party_cfg.Hooks.CanAddToParty = NULL;
	}
	if (party_cfg.Hooks.CanRemoveMember != NULL) {
		delete party_cfg.Hooks.CanRemoveMember;
		party_cfg.Hooks.CanRemoveMember = NULL;
	}
	if (party_cfg.Hooks.CanLeaveParty != NULL) {
		delete party_cfg.Hooks.CanLeaveParty;
		party_cfg.Hooks.CanLeaveParty = NULL;
	}
	if (party_cfg.Hooks.OnPublicChat != NULL) {
		delete party_cfg.Hooks.OnPublicChat;
		party_cfg.Hooks.OnPublicChat = NULL;
	}
	if (party_cfg.Hooks.OnPrivateChat != NULL) {
		delete party_cfg.Hooks.OnPrivateChat;
		party_cfg.Hooks.OnPrivateChat = NULL;
	}
	if (party_cfg.Hooks.OnDisband != NULL) {
		delete party_cfg.Hooks.OnDisband;
		party_cfg.Hooks.OnDisband = NULL;
	}
	if (party_cfg.Hooks.ChangePublicChat != NULL) {
		delete party_cfg.Hooks.ChangePublicChat;
		party_cfg.Hooks.ChangePublicChat = NULL;
	}
	if (party_cfg.Hooks.ChangePrivateChat != NULL) {
		delete party_cfg.Hooks.ChangePrivateChat;
		party_cfg.Hooks.ChangePrivateChat = NULL;
	}
	if (party_cfg.Hooks.OnLeaveParty != NULL) {
		delete party_cfg.Hooks.OnLeaveParty;
		party_cfg.Hooks.OnLeaveParty = NULL;
	}
	if (party_cfg.Hooks.OnAddToParty != NULL) {
		delete party_cfg.Hooks.OnAddToParty;
		party_cfg.Hooks.OnAddToParty = NULL;
	}
	if (party_cfg.Hooks.OnPartyCreate != NULL) {
		delete party_cfg.Hooks.OnPartyCreate;
		party_cfg.Hooks.OnPartyCreate = NULL;
	}
	if (party_cfg.Hooks.OnDecline != NULL) {
		delete party_cfg.Hooks.OnDecline;
		party_cfg.Hooks.OnDecline = NULL;
	}
	if (party_cfg.Hooks.OnLootPermChange != NULL) {
		delete party_cfg.Hooks.OnLootPermChange;
		party_cfg.Hooks.OnLootPermChange = NULL;
	}
}
void unload_party()
{
	unload_party_hooks();

	for( Parties::iterator itr = parties.begin(); itr != parties.end(); ++itr )
	{
		(*itr)->cleanup();
	}
	parties.clear();
}

void Party::cleanup()
{
	_member_serials.clear();
	_candidates_serials.clear();
	_offlinemember_serials.clear();
}

u32 Party::leader()
{
   return _leaderserial;
}

bool Party::is_member(u32 serial)
{
	for (vector<u32>::iterator itr=_member_serials.begin(); itr != _member_serials.end(); ++itr)
	{
		if (*itr==serial)
			return true;
	}
	return false;
}

bool Party::is_candidate(u32 serial)
{
	for (vector<u32>::iterator itr=_candidates_serials.begin(); itr != _candidates_serials.end(); ++itr)
	{
		if (*itr==serial)
			return true;
	}
	return false;
}

bool Party::register_with_members()
{
	vector<u32>::iterator itr = _member_serials.begin();
	while( itr != _member_serials.end() )
	{
        Character* chr = system_find_mobile( *itr );
        if (chr != NULL)
		{
			chr->party( this );
			++itr;
		}
        else
			itr = _member_serials.erase( itr );
	}
	if (_member_serials.empty())
		return(false);
	if (!test_size())
		return(false);
	if (system_find_mobile( _leaderserial )==NULL)
		_leaderserial=*_member_serials.begin();

	return (true);
}

bool Party::is_leader(u32 serial)
{
	return _leaderserial==serial;
}

u32 Party::get_member_at(unsigned short pos)
{
	if (_member_serials.size() < pos)
		return NULL;
	return _member_serials[pos];

}

bool Party::add_candidate(u32 serial)
{
	if ( (_member_serials.size()+_candidates_serials.size()) >= party_cfg.General.MaxPartyMembers )
		return false;
	_candidates_serials.push_back(serial);
	return true;
}

bool Party::add_member(u32 serial)
{
	if ( (_member_serials.size()+_candidates_serials.size()) >= party_cfg.General.MaxPartyMembers )
		return false;
	_member_serials.push_back(serial);
	return true;
}

void Party::add_offline_mem(u32 serial)
{
	_offlinemember_serials.push_back(serial);
}

bool Party::remove_candidate(u32 serial)
{
	for (vector<u32>::iterator itr=_candidates_serials.begin(); itr != _candidates_serials.end(); ++itr)
	{
		if (*itr==serial)
		{
			_candidates_serials.erase(itr);
			return true;
		}
	}
	return false;
}

bool Party::remove_member(u32 serial)
{
	for (vector<u32>::iterator itr=_member_serials.begin(); itr != _member_serials.end(); ++itr)
	{
		if (*itr==serial)
		{
			_member_serials.erase(itr);
			return true;
		}
	}
	return false;
}

bool Party::remove_offline_mem(u32 serial)
{
	for (vector<u32>::iterator itr=_offlinemember_serials.begin(); itr != _offlinemember_serials.end(); ++itr)
	{
		if (*itr==serial)
		{
			_offlinemember_serials.erase(itr);
			return true;
		}
	}
	return false;
}

void Party::set_leader(u32 serial)
{
	for (vector<u32>::iterator itr=_member_serials.begin(); itr != _member_serials.end(); ++itr)
	{
		if (*itr==serial)
		{
			_member_serials.erase(itr);
			break;
		}
	}
	_member_serials.insert(_member_serials.begin(),serial);
	_leaderserial=serial;
}

bool Party::test_size()
{
	if ( ( _candidates_serials.size() == 0 ) && ( _member_serials.size() <= 1 ) )
		return false;
	return true;
}

bool Party::can_add()
{
	if ( (_member_serials.size()+_candidates_serials.size()) >= party_cfg.General.MaxPartyMembers )
		return false;
	return true;
}

void Party::send_member_list(Character* to_chr)
{
	PKTBI_BF msg;
	msg.msgtype=PKTBI_BF_ID;
	
	msg.subcmd=ctBEu16(PKTBI_BF::TYPE_PARTY_SYSTEM);
	msg.partydata.partycmd=PKTBI_BF_06::PARTYCMD_ADD;
	
	u8 i=0;

	vector<u32>::iterator itr = _member_serials.begin();
	while( itr != _member_serials.end() )
	{
        Character* chr = system_find_mobile( *itr );
        if (chr != NULL)
		{
			msg.partydata.partyaddout.serials[i].memberid=chr->serial_ext;
			++i;
			++itr;
		}
        else
			itr = _member_serials.erase( itr );
	}

	msg.partydata.partyaddout.nummembers=i;
	msg.msglen=ctBEu16(7+_member_serials.size()*4);

	if (to_chr==NULL)
	{
		for( vector<u32>::iterator itr = _member_serials.begin(); itr != _member_serials.end(); ++itr)
		{
			Character* chr = system_find_mobile( *itr );
			if (chr != NULL)
			{
				if (chr->has_active_client())
					chr->client->transmit(&msg,cfBEu16(msg.msglen));
			}
		}
	}
	else
		to_chr->client->transmit(&msg,cfBEu16(msg.msglen));
}

void Party::disband()
{
	if (party_cfg.Hooks.OnDisband)
		party_cfg.Hooks.OnDisband->call( CreatePartyRefObjImp( this ) );

	for( vector<u32>::iterator itr = _member_serials.begin(); itr != _member_serials.end(); ++itr)
	{
        Character* chr = system_find_mobile( *itr );
		if (chr != NULL)
        {
			chr->party(NULL);
			if (chr->has_active_client())
			{
				send_empty_party(chr);				
				send_sysmessage_cl(chr->client, CLP_Disbanded); // Your party has disbanded.
			}
        }
	}
	for( vector<u32>::iterator itr = _candidates_serials.begin(); itr != _candidates_serials.end(); ++itr)
	{
        Character* chr = system_find_mobile( *itr );
		if (chr != NULL)
			chr->candidate_of(NULL);
	}
	for( vector<u32>::iterator itr = _offlinemember_serials.begin(); itr != _offlinemember_serials.end(); ++itr)
	{
        Character* chr = system_find_mobile( *itr );
		if (chr != NULL)
			chr->offline_mem_of(NULL);
	}

	_member_serials.clear();
	_candidates_serials.clear();
	_offlinemember_serials.clear();
}

void Party::send_remove_member(Character* remchr, bool *disband)
{
	*disband=false;
	if (remchr->serial == _leaderserial)
	{
		*disband=true;
		return;
	}
	else
	{
		PKTBI_BF msg;
		msg.msgtype=PKTBI_BF_ID;
	
		msg.subcmd=ctBEu16(PKTBI_BF::TYPE_PARTY_SYSTEM);
		msg.partydata.partycmd=PKTBI_BF_06::PARTYCMD_REMOVE;
		msg.partydata.partyremoveout.remmemberid=remchr->serial_ext;
		unsigned int i=0;
		vector<u32>::iterator itr = _member_serials.begin();
		while( itr != _member_serials.end() )
		{
			Character* chr = system_find_mobile( *itr );
			if (chr != NULL)
			{
				msg.partydata.partyremoveout.serials[i].memberid=chr->serial_ext;
				++i;
				++itr;
			}
			else
				itr = _member_serials.erase( itr );
		}
		msg.partydata.partyremoveout.nummembers=static_cast<u8>(_member_serials.size());
		msg.msglen=ctBEu16(11+_member_serials.size()*4);

		for( vector<u32>::iterator itr = _member_serials.begin(); itr != _member_serials.end(); ++itr)
		{
			Character* chr = system_find_mobile( *itr );
			if (chr != NULL)
			{
				if (chr->has_active_client())
					chr->client->transmit(&msg,cfBEu16(msg.msglen));
			}
		}
		send_msg_to_all(CLP_Player_Removed);//A player has been removed from your party.
		if (!test_size())
		{
			send_msg_to_all(CLP_Last_Person); //The last person has left the party...
			*disband=true;
		}
	}
}

void Party::send_msg_to_all(unsigned int clilocnr, const char* affix, Character* exeptchr)
{
	unsigned msglen=0;
	unsigned char* msg_buffer=NULL;
	if (affix!=NULL)
		msg_buffer= build_sysmessage_cl_affix(&msglen,clilocnr, affix,true );
	else
		msg_buffer=build_sysmessage_cl(&msglen, clilocnr);

	for( vector<u32>::iterator itr = _member_serials.begin(); itr != _member_serials.end(); ++itr)
	{
		Character* chr = system_find_mobile( *itr );
		if (chr != NULL)
		{
			if (chr!=exeptchr)
			{
				if (chr->has_active_client())
					chr->client->transmit(msg_buffer,msglen);
			}
		}
	}
	if (msg_buffer!=NULL)
	{
		delete[] msg_buffer;
		msg_buffer=NULL;
	}
}

void Party::send_stat_to(Character* chr,Character* bob)
{
	if ((chr != bob) && (is_member(bob->serial)) && (chr->realm==bob->realm)
		&& (pol_distance(chr->x, chr->y, bob->x, bob->y) < 20 ))
	{
		if (!chr->is_visible_to_me(bob))
			send_short_statmsg( chr->client, bob );

		send_attributes_normalized(chr, bob);
	}
}

void Party::send_stats_on_add(Character* newmember)
{
	for( vector<u32>::iterator itr = _member_serials.begin(); itr != _member_serials.end(); ++itr)
	{
		Character* chr = system_find_mobile( *itr );
		if (chr != NULL)
		{
			if (newmember!=chr)
			{
				if (chr->has_active_client())
				{
					send_short_statmsg( chr->client, newmember );
					send_attributes_normalized(chr,newmember);
				}
				if (newmember->has_active_client())
				{
					send_short_statmsg(newmember->client, chr );
					send_attributes_normalized(newmember,chr);
				}
			}
		}
	}
}

void Party::on_mana_changed(Character* chr)
{
	PKTOUT_A2 msg;
	msg.msgtype = PKTOUT_A2_ID;
	msg.serial = chr->serial_ext;
	long h, mh;

    h = chr->vital(uoclient_general.mana.id).current_ones();
    if (h > 0xFFFF)
		h = 0xFFFF;
	mh = chr->vital(uoclient_general.mana.id).maximum_ones();
    if (mh > 0xFFFF)
        mh = 0xFFFF;
	msg.mana = ctBEu16( static_cast<u16>(h * 1000 / mh) );
	msg.max_mana = ctBEu16( 1000 );

	for( vector<u32>::iterator itr = _member_serials.begin(); itr != _member_serials.end(); ++itr)
	{
		Character* mem = system_find_mobile( *itr );
		if (mem != NULL)
		{
			if (mem!=chr)
			{
				if (mem->has_active_client())
					mem->client->transmit(&msg,9);
			}
		}
	}
}
void Party::on_stam_changed(Character* chr)
{
	PKTOUT_A3 msg;
	msg.msgtype = PKTOUT_A3_ID;
	msg.serial = chr->serial_ext;
	long h, mh;

	h = chr->vital(uoclient_general.stamina.id).current_ones();
    if (h > 0xFFFF)
		h = 0xFFFF;
	mh = chr->vital(uoclient_general.stamina.id).maximum_ones();
    if (mh > 0xFFFF)
        mh = 0xFFFF;
	msg.stamina = ctBEu16( static_cast<u16>(h * 1000 / mh) );
	msg.max_stamina = ctBEu16( 1000 );

	for( vector<u32>::iterator itr = _member_serials.begin(); itr != _member_serials.end(); ++itr)
	{
		Character* mem = system_find_mobile( *itr );
		if (mem != NULL)
		{
			if (mem!=chr)
			{
				if (mem->has_active_client())
					mem->client->transmit(&msg,9);
			}
		}
	}
}

void Party::send_member_msg_public(Character* chr,u16* wtext, size_t wtextlen)
{
	PKTBI_BF msg;
	msg.msgtype=PKTBI_BF_ID;
	
	msg.subcmd=ctBEu16(PKTBI_BF::TYPE_PARTY_SYSTEM);
	msg.partydata.partycmd=PKTBI_BF_06::PARTYCMD_PARTY_MSG;
	msg.partydata.partymsgout.memberid=chr->serial_ext;

	if (party_cfg.Hooks.ChangePublicChat)
	{
		ObjArray* arr;
		if ( !convertUCtoArray(wtext, arr, wtextlen, true) ) // convert back with ctBEu16()
			return;
		BObject obj=party_cfg.Hooks.ChangePublicChat->call_object(chr->make_ref(),arr);
		if (obj->isa(BObjectImp::OTArray))
		{
			arr = static_cast<ObjArray*>(obj.impptr());
			unsigned len = arr->ref_arr.size();
			//wtext[ SPEECH_MAX_LEN+1 ];
			if ( !convertArrayToUC(arr, wtext, len, true) )
				return;
			wtextlen=len+1;
		}
		else if (obj->isa(BObjectImp::OTLong)) // break on return(0)
		{
			if (!obj->isTrue())
				return;
		}
	}
	unsigned short msglen = static_cast<unsigned short>(10+ wtextlen*sizeof(msg.partydata.partymsgout.wtext[0]));
	msg.msglen=ctBEu16(msglen);
	memcpy( msg.partydata.partymsgout.wtext, wtext, wtextlen*sizeof(msg.partydata.partymsgout.wtext[0]) );

	for( vector<u32>::iterator itr = _member_serials.begin(); itr != _member_serials.end(); ++itr)
	{
		Character* mem = system_find_mobile( *itr );
		if (mem != NULL)
		{
			if (mem->has_active_client())
				mem->client->transmit(&msg,msglen);
		}
	}
}

void Party::send_member_msg_private(Character* chr, Character* tochr, u16* wtext, size_t wtextlen)
{
	if (!tochr->has_active_client())
		return;
	PKTBI_BF msg;
	msg.msgtype=PKTBI_BF_ID;
	
	msg.subcmd=ctBEu16(PKTBI_BF::TYPE_PARTY_SYSTEM);
	msg.partydata.partycmd=PKTBI_BF_06::PARTYCMD_MEMBER_MSG;
	msg.partydata.partymsgout.memberid=chr->serial_ext;

	if (party_cfg.Hooks.ChangePrivateChat)
	{
		ObjArray* arr;
		if ( !convertUCtoArray(wtext, arr, wtextlen, true) ) // convert back with ctBEu16()
			return;
		BObject obj=party_cfg.Hooks.ChangePrivateChat->call_object(chr->make_ref(),tochr->make_ref(),arr);
		if (obj->isa(BObjectImp::OTArray))
		{
			arr = static_cast<ObjArray*>(obj.impptr());
			unsigned len = arr->ref_arr.size();
			//wtext[ SPEECH_MAX_LEN+1 ];
			if ( !convertArrayToUC(arr, wtext, len, true) )
				return;
			wtextlen=len+1;
		}
		else if (obj->isa(BObjectImp::OTLong)) // break on return(0)
		{
			if (!obj->isTrue())
				return;
		}
	}
	unsigned short msglen = static_cast<unsigned short>(10+ wtextlen*sizeof(msg.partydata.partymsgout.wtext[0]));
	if (party_cfg.General.PrivateMsgPrefixLen)
	{
		msglen+=static_cast<unsigned short>(party_cfg.General.PrivateMsgPrefixLen*sizeof(msg.partydata.partymsgout.wtext[0]));
		memcpy( msg.partydata.partymsgout.wtext, party_cfg.General.PrivateMsgPrefix, 
			party_cfg.General.PrivateMsgPrefixLen*sizeof(msg.partydata.partymsgout.wtext[0]) );
	}

	msg.msglen=ctBEu16(msglen);

	memcpy( msg.partydata.partymsgout.wtext+party_cfg.General.PrivateMsgPrefixLen, wtext, wtextlen*sizeof(msg.partydata.partymsgout.wtext[0]) );
	tochr->client->transmit(&msg,msglen);
}

void Party::printOn( ostream& os ) const
{
	if ( _member_serials.size() <= 1 )
		return;
	if ( system_find_mobile( _leaderserial ) == NULL )
		return;

    os << "Party" << pf_endl
       << "{" << pf_endl
       << "\tLeader\t0x" << hex << _leaderserial << dec << pf_endl;

	for( vector<u32>::const_iterator itr = _member_serials.begin(); itr != _member_serials.end(); ++itr)
	{
		unsigned long mserial = (*itr);

		Character* mem = system_find_mobile( mserial );
		if (mem != NULL)
			os << "\tMember\t0x" << hex << (mserial) << dec << pf_endl;
	}
	_proplist.printProperties( os );
    os << "}" << pf_endl
       << pf_endl;
}

void send_empty_party(Character* chr)
{
	if (chr != NULL && chr->has_active_client()) {
		PKTBI_BF msg;
		msg.msgtype=PKTBI_BF_ID;
		msg.msglen=ctBEu16(11);
		msg.subcmd=ctBEu16(PKTBI_BF::TYPE_PARTY_SYSTEM);
		msg.partydata.partycmd=PKTBI_BF_06::PARTYCMD_REMOVE;
		msg.partydata.partyemptylist.nummembers=0;
		msg.partydata.partyemptylist.remmemberid=chr->serial_ext;
		chr->client->transmit(&msg,11);
	}
}

void register_party_members()
{
	Parties::iterator itr = parties.begin();
	while( itr != parties.end() )
	{
		if (!(*itr)->register_with_members())
			itr = parties.erase( itr );
		else
			++itr;
	}
}

void disband_party(u32 leader)
{
	for( Parties::iterator itr = parties.begin(); itr != parties.end(); ++itr )
	{
		if ((*itr)->is_leader(leader))
		{
			(*itr)->disband();
			parties.erase(itr);
			break;
		}
	}
}

void read_party_dat()
{
    string partyfile = config.world_data_path + "parties.txt";

	if (!FileExists( partyfile ))
        return;
	if (party_cfg.General.RemoveMemberOnLogoff)
		return;

	cout << "  " << partyfile << ":";
    ConfigFile cf( partyfile );
    ConfigSection sect_party( cf, "PARTY", CST_NORMAL );
    ConfigElem elem;
	static int num_until_dot = 1000;
	clock_t start = clock();
    while (cf.read( elem ))
    {
		if (--num_until_dot == 0)
        {
            cout << ".";
            num_until_dot = 1000;
        }
        if (sect_party.matches( elem ))
			parties.push_back(ref_ptr<Party>(new Party(elem)));
    }
	clock_t end = clock();
	long ms = static_cast<long>((end-start) * 1000.0 / CLOCKS_PER_SEC);

	cout << " " << parties.size() << " elements in " << ms << " ms." << std::endl;

	register_party_members();
}

void write_party( ostream& os )
{
	if (party_cfg.General.RemoveMemberOnLogoff)
		return;
	for( Parties::iterator itr = parties.begin(); itr != parties.end(); ++itr )
    {
		(*itr)->printOn(os);
    }
}



Party* Character::party() const
{
    return party_;
}
void Character::party( Party* p )
{
    party_ = p;
}

bool Character::party_can_loot() const
{
	return party_can_loot_;
}

void Character::set_party_can_loot(bool b)
{
	party_can_loot_ = b;
}

Party* Character::candidate_of() const
{
	return candidate_of_;
}

void Character::candidate_of( Party* c )
{
	candidate_of_ = c;
}

Party* Character::offline_mem_of() const
{
	return offline_mem_of_;
}

void Character::offline_mem_of( Party* c )
{
	offline_mem_of_ = c;
}

void Character::set_party_invite_timeout()
{
	if (this->party_decline_timeout_!=NULL)
		this->party_decline_timeout_->cancel();
	polclock_t timeout=polclock()+party_cfg.General.DeclineTimeout*POLCLOCKS_PER_SEC;
	new OneShotTaskInst<Character*>( &this->party_decline_timeout_,
									 timeout,
									 invite_timeout,
									 this );
}

bool Character::has_party_invite_timeout()
{
	if ( this->party_decline_timeout_ != NULL )
		return false;
	return true;
}

void Character::cancel_party_invite_timeout()
{
	if ( this->party_decline_timeout_ != NULL )
		this->party_decline_timeout_->cancel();
}

void on_loggoff_party(Character* chr)
{
	if (chr->party()!=NULL)
	{
		if (party_cfg.General.RemoveMemberOnLogoff)
		{
			Party* party=chr->party();
			if (party->remove_member(chr->serial))
			{
				bool disband=false;
				if (party->is_leader(chr->serial))
				{
					if (!party->test_size())
					{
						disband_party(party->leader());
						return;
					}
					party->set_leader(party->get_member_at(0));
					party->send_msg_to_all(CLP_Player_Removed);//A player has been removed from your party.
					party->send_member_list(NULL);
				}
				else
					party->send_remove_member(chr,&disband);
				chr->party(NULL);
				if (party_cfg.Hooks.OnLeaveParty)
					party_cfg.Hooks.OnLeaveParty->call(chr->make_ref(), chr->make_ref() );

				if (disband)
					disband_party(party->leader());
				else
				{
					if (party_cfg.General.RejoinPartyOnLogon)
					{
						party->add_offline_mem(chr->serial);
						chr->offline_mem_of(party);
					}
				}
			}
		}
	}
}


void on_loggon_party(Character* chr)
{
	if (chr->offline_mem_of()!=NULL)
	{
		Party* party = chr->offline_mem_of();
		if (party->remove_offline_mem(chr->serial))
		{
			if (party->add_member(chr->serial))
			{
				chr->party(party);
				send_sysmessage_cl(chr->client, CLP_Rejoined); //You have rejoined the party.
				party->send_member_list(NULL);
				party->send_stats_on_add(chr);
				party->send_msg_to_all(CLP_Player_Rejoined,chr->name().c_str(),chr); //: rejoined the party.
			}
		}
		chr->offline_mem_of(NULL);
	}
	else if (chr->party()==NULL)
		send_empty_party(chr);
	else
	{
		send_sysmessage_cl(chr->client, CLP_Rejoined); //You have rejoined the party.
		chr->party()->send_member_list(chr);
		chr->party()->send_stats_on_add(chr);
		chr->party()->send_msg_to_all(CLP_Player_Rejoined,chr->name().c_str(),chr); //: rejoined the party.
	}
}

void handle_unknown_packet( Client* client );
class Party;
void party_cmd_handler( Client* client, PKTBI_BF* msg )
{
	switch(msg->partydata.partycmd)
	{
	case PKTBI_BF_06::PARTYCMD_ADD:
		handle_add(client,msg);
		break;
	case PKTBI_BF_06::PARTYCMD_REMOVE:
		handle_remove(client,msg);
		break;
	case PKTBI_BF_06::PARTYCMD_MEMBER_MSG:
		handle_member_msg(client,msg);
		break;
	case PKTBI_BF_06::PARTYCMD_PARTY_MSG:
		handle_party_msg(client,msg);
		break;
	case PKTBI_BF_06::PARTYCMD_LOOT_PERMISSION:
		handle_loot_perm(client,msg);
		break;
	case PKTBI_BF_06::PARTYCMD_ACCEPT_INVITE:
		handle_accept_invite(client,msg);
		break;
	case PKTBI_BF_06::PARTYCMD_DECLINE_INVITE:
		handle_decline_invite(client,msg);
		break;
	default:
		handle_unknown_packet( client );
	}
}
FullMsgTargetCursor add_member_cursor( handle_add_member_cursor );
void handle_add_member_cursor( Character* chr, PKTBI_6C* msgin )
{
	if (chr->client != NULL)
	{
		Character* add=system_find_mobile(cfBEu32(msgin->selected_serial));
		add_candidate(add,chr);
	}
}

void handle_add( Client* client, PKTBI_BF* msg )
{
	if (msg->partydata.partyadd.memberid==0x0)
	{
		send_sysmessage_cl(client, CLP_Who_To_Add); //Who would you like to add to your party?
		add_member_cursor.send_object_cursor( client );
	}
	else
	{
		Character* member = system_find_mobile(cfBEu32(msg->partydata.partyadd.memberid));
		add_candidate(member,client->chr);
	}
}
FullMsgTargetCursor remove_member_cursor( handle_remove_member_cursor );
void handle_remove_member_cursor( Character* chr, PKTBI_6C* msgin )
{
	if (chr->client != NULL)
	{
		Character* rem=system_find_mobile(cfBEu32(msgin->selected_serial));
		if (rem!=NULL)
		{
			Party* party=chr->party();
			if ((party==NULL) || (!party->is_leader(chr->serial)) || (!party->is_member(rem->serial)))
				return;
			if (chr==rem)
				send_sysmessage_cl(chr->client, CLP_Cannot_Remove_Self); //You may only remove yourself from a party if you are not the leader.
			else
			{
				if (party_cfg.Hooks.CanRemoveMember)
				{
					if ( !party_cfg.Hooks.CanRemoveMember->call(chr->make_ref(), rem->make_ref()) )
					{
						party->send_member_list(chr); //resend list
						return;
					}
				}
				if (party->remove_member(rem->serial))
				{
					bool disband;
					party->send_remove_member(rem,&disband);
					rem->party(NULL);
					if (party_cfg.Hooks.OnLeaveParty)
						party_cfg.Hooks.OnLeaveParty->call(rem->make_ref(), chr->make_ref() );

					send_sysmessage_cl(rem->client, CLP_Removed); //You have been removed from the party.
					send_empty_party(rem);
					if (disband)
						disband_party(party->leader());
				}
			}
		}
	}
}

void handle_remove( Client* client, PKTBI_BF* msg )
{
	Party* party=client->chr->party();
	if (party==NULL)
	{
		send_sysmessage_cl(client, CLP_No_Party); //You are not in a party.
		return;
	}
	Character* member = system_find_mobile(cfBEu32(msg->partydata.partyremove.memberid));
	// leader wants target
	if ((party->is_leader(client->chr->serial)) && (member==NULL))
	{
		send_sysmessage_cl(client, CLP_Who_to_Remove); //Who would you like to remove from your party?
		remove_member_cursor.send_object_cursor(client);
	}
	// leader kicks member, or member kicks himself
	else if ( ((party->is_leader(client->chr->serial)) || (client->chr==member)) && (party->is_member(member->serial)))
	{
		if (client->chr==member)
		{
			if (party_cfg.Hooks.CanLeaveParty)
			{
				if ( !party_cfg.Hooks.CanLeaveParty->call(client->chr->make_ref()) )
				{
					party->send_member_list(client->chr); //resend list
					return;
				}
			}
		}
		else
		{
			if (party_cfg.Hooks.CanRemoveMember)
			{
				if ( !party_cfg.Hooks.CanRemoveMember->call(client->chr->make_ref(),member->make_ref()) )
				{
					party->send_member_list(client->chr); //resend list
					return;
				}
			}
		}
		if (party->remove_member(member->serial))
		{
			bool disband;
			member->party(NULL);
			party->send_remove_member(member,&disband);
			if (party_cfg.Hooks.OnLeaveParty)
				party_cfg.Hooks.OnLeaveParty->call(member->make_ref(), client->chr->make_ref() );
			if (member->has_active_client())
			{
				send_sysmessage_cl(member->client, CLP_Removed); //You have been removed from the party.
				send_empty_party(member);
			}
			if (disband)
				disband_party(party->leader());
		}
	}
}

void handle_member_msg( Client* client, PKTBI_BF* msg )
{
	using std::wcout;
	Character* member = system_find_mobile(cfBEu32(msg->partydata.partymembermsg.memberid));
	if (member !=NULL)
	{
		Party* party=client->chr->party();

		if ((party!=NULL) && (member->party()!=NULL) && (party->is_member(member->serial)))
		{
			int intextlen;
			u16 * themsg = msg->partydata.partymembermsg.wtext;
			//u8 *  bytemsg = ((u8 *) themsg);
			//int wtextoffset = 0;
			int i;

			u16 wtextbuf[ SPEECH_MAX_LEN+1 ];
			size_t wtextbuflen;

			intextlen = (cfBEu16(msg->msglen) - 10)
						/ sizeof(msg->partydata.partymembermsg.wtext[0]) - 1;

			//	intextlen does not include the null terminator.

			// Preprocess the text into a sanity-checked, printable, null-terminated form in textbuf
			if (intextlen < 0)
				intextlen = 0;
			if (intextlen > SPEECH_MAX_LEN) 
				intextlen = SPEECH_MAX_LEN;

			wtextbuflen = 0;
			for( i = 0; i < intextlen; i++ )
			{
				u16 wc = cfBEu16(themsg[i]);
				if (wc == 0) break;		// quit early on embedded nulls
				if (wc == L'~') continue;	// skip unprintable tildes. 
				wtextbuf[ wtextbuflen++ ] = ctBEu16(wc);
			}
			wtextbuf[ wtextbuflen++ ] = (u16)0;

			if (party_cfg.Hooks.OnPrivateChat)
			{
				ObjArray* arr;
				if ( convertUCtoArray(wtextbuf, arr, wtextbuflen,true) ) // convert back with ctBEu16()
					party_cfg.Hooks.OnPrivateChat->call(client->chr->make_ref(),member->make_ref(),arr);
			}
			
			party->send_member_msg_private(client->chr,member,wtextbuf,wtextbuflen);
		}
		else
			send_sysmessage_cl(client, CLP_No_Party);//You are not in a party.
	}
}

void handle_party_msg( Client* client, PKTBI_BF* msg )
{
	using std::wcout; // wcout.narrow() function r0x! :-)
	if (client->chr->party()!=NULL)
	{
		int intextlen;
		u16 * themsg = msg->partydata.partymsg.wtext;
		//u8 *  bytemsg = ((u8 *) themsg);
		//int wtextoffset = 0;
		int i,starti;
		Character* member = NULL;

		u16 wtextbuf[ SPEECH_MAX_LEN+1 ];
		size_t wtextbuflen;

		intextlen = (cfBEu16(msg->msglen) - 6)
						/ sizeof(msg->partydata.partymsg.wtext[0]) - 1;

		//	intextlen does not include the null terminator.

		// Preprocess the text into a sanity-checked, printable, null-terminated form in textbuf
		if (intextlen < 0)
			intextlen = 0;
		if (intextlen > SPEECH_MAX_LEN) 
			intextlen = SPEECH_MAX_LEN;

		wtextbuflen = 0;
		starti=0;
		if (party_cfg.General.TreatNoAsPrivate)
		{
			char no_c=wcout.narrow((wchar_t)cfBEu16(themsg[0]), '?');
			if ((isdigit(no_c)) && (cfBEu16(themsg[1])== L' '))
			{
				int no =atoi(&no_c);
				if (no==0)
					no=9;
				else
					no--;
				u32 mem=client->chr->party()->get_member_at((unsigned short)no);
				if (mem!=0)
				{
					member = system_find_mobile(mem);
					if ((member !=NULL) && (member->has_active_client()))
						starti=2;
				}
			}
		}

		for( i = starti; i < intextlen; i++ )
		{
			u16 wc = cfBEu16(themsg[i]);
			if (wc == 0) break;		// quit early on embedded nulls
			if (wc == L'~') continue;	// skip unprintable tildes. 
			wtextbuf[ wtextbuflen++ ] = ctBEu16(wc);
		}
		wtextbuf[ wtextbuflen++ ] = (u16)0;

		if (starti==2) //private chat
		{
			if (party_cfg.Hooks.OnPrivateChat)
			{
				ObjArray* arr;
				if ( convertUCtoArray(wtextbuf, arr, wtextbuflen,true)) // convert back with ctBEu16()
					party_cfg.Hooks.OnPrivateChat->call(client->chr->make_ref(),member->make_ref(),arr);
			}
			client->chr->party()->send_member_msg_private(client->chr,member,wtextbuf,wtextbuflen);
		}
		else
		{
			if (party_cfg.Hooks.OnPublicChat)
			{
				ObjArray* arr;
				if ( convertUCtoArray(wtextbuf, arr, wtextbuflen, true) ) // convert back with ctBEu16()
					party_cfg.Hooks.OnPublicChat->call(client->chr->make_ref(),arr);
			}

			client->chr->party()->send_member_msg_public(client->chr,wtextbuf,wtextbuflen);
		}
	}
	else
		send_sysmessage_cl(client, CLP_No_Party);//You are not in a party.
}

void handle_loot_perm( Client* client, PKTBI_BF* msg )
{
	if (client->chr->party()==NULL)
		send_sysmessage_cl(client, CLP_No_Party); //You are not in a party.
	else
	{
		bool loot = msg->partydata.partylootperm.canloot ? true: false;
		client->chr->set_party_can_loot(loot);
		if (party_cfg.Hooks.OnLootPermChange)
				party_cfg.Hooks.OnLootPermChange->call( client->chr->make_ref() );
		//You have chosen to allow your party to loot your corpse.
		//You have chosen to prevent your party from looting your corpse.
		send_sysmessage_cl(client, loot ? CLP_Allow_Loot : CLP_Prevent_Loot);
	}
}

void handle_accept_invite( Client* client, PKTBI_BF* msg )
{
	Character* leader = system_find_mobile(cfBEu32(msg->partydata.partyaccinvite.leaderid));
	if (leader != NULL)
	{
		if (leader->party() != NULL)
		{
			Party* party=leader->party();
			if (party->remove_candidate(client->chr->serial))
			{
				client->chr->cancel_party_invite_timeout();
				client->chr->candidate_of(NULL);
				if (party->add_member(client->chr->serial))
				{
					client->chr->party(party);
					if (party_cfg.Hooks.OnAddToParty)
						party_cfg.Hooks.OnAddToParty->call( client->chr->make_ref() );
					send_sysmessage_cl(client, CLP_Added); // You have been added to the party.

					party->send_msg_to_all(CLP_Joined,client->chr->name().c_str(),client->chr);//  : joined the party.
					party->send_member_list(NULL);
					party->send_stats_on_add(client->chr);
				}
			}
		}
		else
			send_sysmessage_cl(client, CLP_No_Invite); //No one has invited you to be in a party.
	}
}

void handle_decline_invite( Client* client, PKTBI_BF* msg )
{
	Character* leader = system_find_mobile(cfBEu32(msg->partydata.partydecinvite.leaderid));
	if (leader != NULL)
	{
		if (leader->party() != NULL)
		{
			Party* party=leader->party();
			if (party->remove_candidate(client->chr->serial))
			{
				client->chr->cancel_party_invite_timeout();
				client->chr->candidate_of(NULL);
				send_sysmessage_cl(client, CLP_Decline); // You notify them that you do not wish to join the party.
				if (leader->has_active_client())
					send_sysmessage_cl_affix(leader->client, CLP_Notify_Decline, client->chr->name().c_str(),true); //: Does not wish to join the party.
				if (party_cfg.Hooks.OnDecline)
					party_cfg.Hooks.OnDecline->call( client->chr->make_ref() );
				if (!party->test_size())
					disband_party(leader->serial);
			}
			else if (party->is_member(client->chr->serial))
				send_sysmessage_cl(client, CLP_Too_Late_Decline); // Too late to decline, you are already in the party. Try /quit.
		}
		else
			send_sysmessage_cl(client, CLP_No_Invite); //No one has invited you to be in a party.
	}
}

void add_candidate(Character* member, Character* leader)
{
	if (member==NULL)
		send_sysmessage_cl(leader->client,CLP_Add_Living); //You may only add living things to your party!
	else if (member==leader)
		send_sysmessage_cl(leader->client,CLP_Add_Yourself); //You cannot add yourself to a party.
	else if ((leader->party() != NULL) && (!leader->party()->is_leader(leader->serial)))
		send_sysmessage_cl(leader->client,CLP_Add_No_Leader); //You may only add members to the party if you are the leader.
	else if ((leader->party() != NULL) && (!leader->party()->can_add()))
		send_sysmessage_cl(leader->client,CLP_Max_Size); //You may only have 10 in your party (this includes candidates).
	else if (member->isa(UObject::CLASS_NPC))
		send_sysmessage_cl(leader->client,CLP_Ignore_Offer); //The creature ignores your offer.
	else if ((leader->party()!=NULL) && (leader->party()->is_member(member->serial)))
		send_sysmessage_cl(leader->client,CLP_Already_Your_Party); //This person is already in your party!
	else if ((leader->party()!=NULL) && (leader->party()->is_candidate(member->serial)))
		send_sysmessage_cl(leader->client,CLP_Already_Your_Party); //This person is already in your party!
	else if (member->party() != NULL)
		send_sysmessage_cl(leader->client,CLP_Already_in_a_Party); //This person is already in a party!
	else
	{
		if (member->candidate_of() != NULL)
			send_sysmessage_cl(leader->client,CLP_Already_in_a_Party); //This person is already in a party!
		else
		{
			if ( party_cfg.Hooks.CanAddToParty )
			{
				if ( !party_cfg.Hooks.CanAddToParty->call(leader->make_ref(), member->make_ref()) )
					return;
			}
			if (party_cfg.General.DeclineTimeout > 0 )
				member->set_party_invite_timeout();

			if (leader->party()==NULL)
			{
				Party* party = new Party(leader->serial);
				parties.push_back(ref_ptr<Party>(party));
				leader->party(party);
				if (party_cfg.Hooks.OnPartyCreate)
					party_cfg.Hooks.OnPartyCreate->call( CreatePartyRefObjImp( party ) );
			}
			if (leader->party()->add_candidate(member->serial))
			{
				member->candidate_of(leader->party());
				send_invite(member,leader);
			}
			else
				send_sysmessage_cl(leader->client,CLP_Max_Size); //You may only have 10 in your party (this includes candidates).
		}
	}
}

void invite_timeout(Character* mem)
{
	Party* party = mem->candidate_of();
	if ( party != NULL )
	{
		if (party->remove_candidate(mem->serial))
		{
			if (mem->has_active_client())
				send_sysmessage_cl(mem->client, CLP_Decline); // You notify them that you do not wish to join the party.
			Character* leader = system_find_mobile(party->leader());
			if (party_cfg.Hooks.OnDecline)
				party_cfg.Hooks.OnDecline->call( mem->make_ref() );
			if (leader!=NULL)
			{
				if (leader->has_active_client())
					send_sysmessage_cl_affix(leader->client, CLP_Notify_Decline, mem->name().c_str(),true); //: Does not wish to join the party.
				if (!party->test_size())
					disband_party(leader->serial);
			}
		}
		mem->candidate_of(NULL);
	}
}

void send_invite(Character* member,Character* leader)
{
	PKTBI_BF msg;
	msg.msgtype=PKTBI_BF_ID;
	msg.msglen=ctBEu16(10);
	msg.subcmd=ctBEu16(PKTBI_BF::TYPE_PARTY_SYSTEM);
	msg.partydata.partycmd=PKTBI_BF_06::PARTYCMD_INVITE_MEMBER;
	msg.partydata.invitemember.leaderid=leader->serial_ext;
	member->client->transmit(&msg,10);

	// : You are invited to join the party. Type /accept to join or /decline to decline the offer.
	send_sysmessage_cl_affix(member->client, CLP_Invite, leader->name().c_str(),true);
	send_sysmessage_cl(leader->client,CLP_Invited); // You have invited them to join the party.
}

void send_attributes_normalized(Character* chr, Character* bob)
{
	PKTOUT_2D msg;
	msg.msgtype=PKTOUT_2D_ID;
	msg.serial=bob->serial_ext;
	long h, mh;

    h = bob->vital(uoclient_general.hits.id).current_ones();
    if (h > 0xFFFF)
		h = 0xFFFF;
    mh = bob->vital(uoclient_general.hits.id).maximum_ones();
    if (mh > 0xFFFF)
        mh = 0xFFFF;
	msg.hitscurrent = ctBEu16( static_cast<u16>(h * 1000 / mh) );
	msg.hitsmax = ctBEu16( 1000 );

	h = bob->vital(uoclient_general.mana.id).current_ones();
    if (h > 0xFFFF)
        h = 0xFFFF;
	mh = bob->vital(uoclient_general.mana.id).maximum_ones();
    if (mh > 0xFFFF)
        mh = 0xFFFF;
	msg.manacurrent = ctBEu16( static_cast<u16>(h * 1000 / mh) );
	msg.manamax = ctBEu16( 1000 );

	h = bob->vital(uoclient_general.stamina.id).current_ones();
    if (h > 0xFFFF)
        h = 0xFFFF;
	mh = bob->vital(uoclient_general.stamina.id).maximum_ones();
    if (mh > 0xFFFF)
        mh = 0xFFFF;
	msg.stamcurrent = ctBEu16( static_cast<u16>(h * 1000 / mh) );
	msg.stammax = ctBEu16( 1000 );

	chr->client->transmit(&msg, 17);
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