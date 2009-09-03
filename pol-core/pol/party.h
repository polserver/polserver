/*
History
=======


Notes
=======

*/

#include "pktboth.h"

void handle_add( Client* client, PKTBI_BF* msg );
void handle_remove( Client* client, PKTBI_BF* msg );
void handle_member_msg( Client* client, PKTBI_BF* msg );
void handle_party_msg( Client* client, PKTBI_BF* msg );
void handle_loot_perm( Client* client, PKTBI_BF* msg );
void handle_accept_invite( Client* client, PKTBI_BF* msg );
void handle_decline_invite( Client* client, PKTBI_BF* msg );

#ifndef PARTY_H

class Character;
class Client;
class NPC;

class Party : public ref_counted
{
	public:
		explicit Party( ConfigElem& elem );
        explicit Party( u32 serial );
		bool is_leader(u32 serial);
		bool is_member(u32 serial);
		bool is_candidate(u32 serial);
		bool register_with_members();
		bool add_candidate(u32 serial);
		bool add_member(u32 serial);
		void add_offline_mem(u32 serial);
		bool remove_candidate(u32 serial);
		bool remove_member(u32 serial);
		bool remove_offline_mem(u32 serial);
		void set_leader(u32 serial);
		void send_remove_member(Character* chr, bool* disband);
		void send_member_list(Character* to_chr);
		void send_msg_to_all(unsigned int clilocnr, const char* affix=0,Character* exeptchr=NULL);
		bool test_size();
		bool can_add();
		void disband();
		void send_stat_to(Character* chr,Character* bob);
		void send_stats_on_add(Character* newmember);
		void on_mana_changed(Character* chr);
		void on_stam_changed(Character* chr);
		void send_member_msg_public(Character* chr,u16* wtext, size_t wtextlen);
		void send_member_msg_private(Character* chr,Character* tochr, u16* wtext, size_t wtextlen);
		u32 leader();
		void printOn( ostream& os ) const;
		u32 get_member_at(unsigned short pos);
		void cleanup();

		friend class EPartyRefObjImp;

	private:
		vector<u32> _member_serials;
		vector<u32> _candidates_serials;
		vector<u32> _offlinemember_serials;
		u32 _leaderserial;
		PropertyList _proplist;
};

#include "bscript/execmodl.h"

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

void send_invite(Character* member,Character* leader);
void handle_add_member_cursor( Character* chr, PKTBI_6C* msgin );
void handle_remove_member_cursor( Character* chr, PKTBI_6C* msgin );
void add_candidate(Character* member, Character* leader);
void disband_party(u32 leader);
void send_empty_party(Character* chr);
void on_loggon_party(Character* chr);
void on_loggoff_party(Character* chr);
void send_attributes_normalized(Character* chr, Character* bob);
void invite_timeout(Character* mem);

#endif
