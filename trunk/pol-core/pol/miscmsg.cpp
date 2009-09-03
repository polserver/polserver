/*
History
=======
2005/11/25 MuadDib:   Added PKTBI_BF::TYPE_SESPAM: to do away with spam.
2005/11/23 MuadDib:   Altered handle_mode_set for 0x72 Packet. Nows reads
                      from combat.cfg for warmode_wait object. Sends the
                      0x77 Packet and returns 0 when out of timer.
2005/12/09 MuadDib:   Added TYPE_CLIENT_LANGUAGE for setting member uclang.
2006/05/24 Shinigami: Added PKTBI_BF::TYPE_CHARACTER_RACE_CHANGER to support Elfs
2006/05/30 Shinigami: Changed params of character_race_changer_handler()
2009/07/23 MuadDib:   updates for new Enum::Packet IDs
2009/08/14 Turley:    Added PKTBI_BF::TYPE_SCREEN_SIZE & TYPE_CLOSED_STATUS_GUMP (anti spam)

Notes
=======

*/

#include "clib/stl_inc.h"

/* MISCMSG.CPP: Miscellaneous message handlers.  Handlers shouldn't stay here long, 
   only until they find a better home - but this is better than putting them in POL.CPP. */

#include <stddef.h>

#include "clib/clib.h"
#include "clib/endian.h"
#include "clib/fdump.h"
#include "clib/logfile.h"
#include "clib/strutil.h"

#include "clib/stlutil.h"

#include "account.h"
#include "charactr.h"
#include "client.h"
#include "cmbtcfg.h"
#include "customhouses.h"
#include "fnsearch.h"
#include "msghandl.h"
#include "multi.h"
#include "pktboth.h"
#include "pktin.h"
#include "spells.h"
#include "tooltips.h"
#include "uvars.h"
#include "ufunc.h"
#include "sockio.h"

#include "ssopt.h"
#include "scrstore.h"
#include "polcfg.h"

void handle_unknown_packet( Client* client );
void character_race_changer_handler( Client* client, PKTBI_BF* msg );
void party_cmd_handler( Client* client, PKTBI_BF* msg );

void OnGuildButton( Client* client );
void OnQuestButton( Client* client );
void OnChatButton( Client* client );

void handle_bulletin_boards( Client* client, PKTBI_71* msg )
{
	handle_unknown_packet( client );
}
MESSAGE_HANDLER_VARLEN( PKTBI_71, handle_bulletin_boards );

void handle_mode_set( Client *client, PKTBI_72 *msg )
{
	if ( client->chr->warmode_wait > read_gameclock() )
	{
        send_move( client, client->chr );
		return;
	}
	else
	{
		client->chr->warmode_wait = read_gameclock() + combat_config.warmode_delay;
	}

	bool msg_warmode = msg->warmode ? true : false;

	// FIXME: Should reply with 0x77 packet!? (so says various docs!) [TJ]
	transmit( client, msg, sizeof *msg );

    client->chr->set_warmode( msg_warmode );
}
MESSAGE_HANDLER( PKTBI_72, handle_mode_set );

void handle_rename_char( Client* client, PKTIN_75* msg )
{
    Character* chr = find_character( cfBEu32( msg->serial ));
    if (chr != NULL)
    {
        if (client->chr->can_rename( chr ))
        {
            msg->name[ sizeof msg->name-1 ] = '\0';
            // check for legal characters
			for( char* p = msg->name; *p; p++ )
			{
				// only allow: a-z, A-Z & spaces
				if ( *p != ' ' && !isalpha(*p) )
				{
					char buffer[512];
					sprintf(buffer, "Client #%lu (account %s) attempted an invalid rename (packet 0x%2.02x):",
									client->instance_,
									(client->acct != NULL) ? client->acct->name() : "unknown",
									msg->msgtype);
					cout << buffer << endl;
					fdump( stdout, msg->name, static_cast<int>(strlen(msg->name)) );

					if (logfile)
					{
						Log("%s\n", buffer);
						fdump( logfile, msg->name, static_cast<int>(strlen(msg->name)) );
					}

					*p = '\0';
					send_sysmessage( client, "Invalid name!" );
					return; //dave 12/26 if invalid name, do not apply to chr!
				}
			}
            chr->setname( msg->name );
        }
        else
        {
            send_sysmessage( client, "I can't rename that." );
        }
    }
    else
    {
        send_sysmessage( client, "I can't find that." );
    }
}
MESSAGE_HANDLER( PKTIN_75, handle_rename_char );

void handle_msg_B5( Client* client, PKTIN_B5* msg )
{
    OnChatButton( client );
}
MESSAGE_HANDLER( PKTIN_B5, handle_msg_B5 );

void handle_msg_B8( Client* client, PKTBI_B8* msg )
{
    handle_unknown_packet( client );
}
MESSAGE_HANDLER_VARLEN( PKTBI_B8, handle_msg_B8 );

void handle_msg_BB( Client* client, PKTBI_BB* msg )
{
    handle_unknown_packet( client );
}
MESSAGE_HANDLER( PKTBI_BB, handle_msg_BB );

void handle_client_version( Client* client, PKTBI_BD* msg )
{
	u16 len = cfBEu16(msg->msglen)-3;
	if (len < 100)
	{
		int c=0;
		char ch;
		string ver2 ="";
		while (c<len)
		{
			ch = msg->version[c];
			if (ch==0) break; // seems to be null-terminated
			ver2+=ch;
			++c;
		} 
		client->setversion( ver2 );

		VersionDetailStruct vers_det;
		client->itemizeclientversion(ver2, vers_det);
		client->setversiondetail(vers_det);

		if (client->compareVersion(CLIENT_VER_60171)) //Grid-loc support
			client->is_greq_6017=true;
		send_season_info( client );	// Scott 10/11/2007 added for login fixes and handling 1.x clients.
									// Season info needs to check client version to keep from crashing 1.x
									// version not set until shortly after login complete.
		//send_feature_enable(client); //dave commented out 8/21/03, unexpected problems with people sending B9 via script with this too.
		if( (client->UOExpansionFlag & AOS) )
		{
		send_object_cache(client, dynamic_cast<const UObject*>(client->chr));
		}
	}
	else
	{
		Log2( "Suspect string length in PKTBI_BD packet: %u\n", (unsigned) len );
	}
}
MESSAGE_HANDLER_VARLEN( PKTBI_BD, handle_client_version );

void ext_stats_in (Client* client, PKTBI_BF* msg)
{
	if (ssopt.core_handled_locks) {
		const Attribute *attrib = NULL;
		switch (msg->extstatin.stat) {
			case PKTBI_BF_1A::STAT_STR:
				attrib = pAttrStrength; 
				break;
			case PKTBI_BF_1A::STAT_DEX:
				attrib = pAttrDexterity;
				break;
			case PKTBI_BF_1A::STAT_INT:
				attrib = pAttrIntelligence;
				break;
			default: // sent an illegal stat. Should report to console?
				return;
		}

		if (attrib == NULL) // there's no attribute for this (?)
			return;

		u8 state = msg->extstatin.mode;
		if (state > 2) // FIXME hard-coded value
			return;

		client->chr->attribute(attrib->attrid).lock(state);
	}
}

void handle_msg_BF( Client* client, PKTBI_BF* msg )
{
    UObject* obj = NULL;
    UMulti* multi = NULL;
    UHouse* house = NULL;
    switch(cfBEu16(msg->subcmd))
    {
		case PKTBI_BF::TYPE_CLIENT_LANGUAGE:
			client->chr->uclang = strlower(msg->client_lang);
			break;
        case PKTBI_BF::TYPE_REQ_FULL_CUSTOM_HOUSE:
            if( (client->UOExpansionFlag & AOS) == 0 )
                return;
            multi = system_find_multi(cfBEu32(msg->reqfullcustomhouse.house_serial));
            if(multi != NULL)
            {
                house = multi->as_house();
                if(house != NULL)
                {
					if(client->UOExpansionFlag & AOS)
					{
						send_object_cache(client, (UObject*)(house));
					}
                    //consider sending working design to certain players, to assist building, or GM help
                    CustomHousesSendFull( house, client, HOUSE_DESIGN_CURRENT );
                }
            }
            break;
        case PKTBI_BF::TYPE_OBJECT_CACHE:
            if( (client->UOExpansionFlag & AOS) == 0)
                return;
            obj = system_find_object(cfBEu32(msg->objectcache.serial));
            if(obj != NULL)
            {
                SendAOSTooltip(client,obj);
            }
            break;
		case PKTBI_BF::TYPE_SESPAM:
			return;
			break;
		case PKTBI_BF::TYPE_SPELL_SELECT:
			do_cast(client, cfBEu16(msg->spellselect.selected_spell) );
			break;
		case PKTBI_BF::TYPE_CHARACTER_RACE_CHANGER:
			character_race_changer_handler( client, msg );
			break;
		case PKTBI_BF::TYPE_PARTY_SYSTEM:
			party_cmd_handler( client, msg );
			break;
		case PKTBI_BF::TYPE_EXTENDED_STATS_IN:
			ext_stats_in(client, msg);
			break;
		case PKTBI_BF::TYPE_CLOSED_STATUS_GUMP:
			return;
			break;
		case PKTBI_BF::TYPE_SCREEN_SIZE:
			return;
			break;
		default:
            handle_unknown_packet( client );
    }
}
MESSAGE_HANDLER_VARLEN( PKTBI_BF, handle_msg_BF );

void handle_unknown_C4( Client* client, PKTOUT_C4* msg )
{
	handle_unknown_packet( client );
}
MESSAGE_HANDLER( PKTOUT_C4, handle_unknown_C4 );

void handle_update_range_change( Client* client, PKTBI_C8* msg )
{
	handle_unknown_packet( client );
}
MESSAGE_HANDLER( PKTBI_C8, handle_update_range_change );

void handle_allnames( Client *client, PKTBI_98_IN *msg )
{

	u32 serial = cfBEu32( msg->serial );
	Character *the_mob = find_character( serial );
	if (the_mob != NULL)
	{
		if (!client->chr->is_visible_to_me(the_mob)) {
			return;
		}
		if (pol_distance(client->chr->x, client->chr->y, the_mob->x, the_mob->y) > 20) {
			return;
		}

		PKTBI_98_OUT allnames;
		allnames.msgtype = PKTBI_98_OUT_ID;
		allnames.msglen = ctBEu16(0x25);  // 0x25 = 37. Static Length.
		allnames.serial = the_mob->serial_ext;
		strzcpy( allnames.name, the_mob->name().c_str(), sizeof allnames.name );
		transmit( client, &allnames, sizeof allnames );

	}
	else
	{
		return;
	}
}
MESSAGE_HANDLER( PKTBI_98_IN, handle_allnames );

void handle_se_object_list( Client* client, PKTBI_D6_IN* msgin )
{
	UObject* obj = NULL;
	int length=cfBEu16(msgin->msglen)-3;
	if ( length < 0 || (length%4) != 0 )
		return;
	int count = length/4;
	
	for( int i = 0; i < count; ++i )
	{
		obj = system_find_object(cfBEu32( msgin->serials[i].serial ));
		if (obj != NULL)
			SendAOSTooltip(client,obj);
	}
}
MESSAGE_HANDLER_VARLEN( PKTBI_D6_IN, handle_se_object_list );

void handle_ef_seed( Client *client, PKTIN_EF *msg )
{
	VersionDetailStruct detail;
	detail.major=cfBEu32(msg->ver_Major);
	detail.minor=cfBEu32(msg->ver_Minor);
	detail.rev=cfBEu32(msg->ver_Revision);
	detail.patch=cfBEu32(msg->ver_Patch);
	client->setversiondetail(detail);
	if (client->compareVersion(CLIENT_VER_60171)) //Grid-loc support
		client->is_greq_6017=true;

	// detail->patch is since 5.0.7 always numeric, so no need to make it complicated
	OSTRINGSTREAM os;
	os << detail.major << "." << detail.minor << "." << detail.rev << "." << detail.patch;
	client->setversion(OSTRINGSTREAM_STR(os));
}
MESSAGE_HANDLER( PKTIN_EF, handle_ef_seed );

void handle_aos_commands (Client *client, PKTBI_D7* msg)
{
	//cout <<" Sizeof: " << sizeof(PKTBI_D7) << endl;
	//cout << "Received AOS command: " << cfBEu16(msg->msglen) << " " << cfBEu16(msg->subcmd) << endl;
	//fdump (stdout, client->buffer, client->bytes_received);

	//should check if serial is valid? client->chr->serial == msg->serial?

	switch (cfBEu16(msg->subcmd)) {
		case PKTBI_D7::CUSTOM_HOUSE_BACKUP:
			CustomHousesBackup(msg);
			break;
		
		case PKTBI_D7::CUSTOM_HOUSE_RESTORE:
			CustomHousesRestore(msg);
			break;
		
		case PKTBI_D7::CUSTOM_HOUSE_COMMIT:
			CustomHousesCommit(msg);
			break;
		
		case PKTBI_D7::CUSTOM_HOUSE_ERASE:
			CustomHousesErase(msg);
			break;
		
		case PKTBI_D7::CUSTOM_HOUSE_ADD:
			CustomHousesAdd(msg);
			break;
		
		case PKTBI_D7::CUSTOM_HOUSE_QUIT:
			CustomHousesQuit(msg);
			break;

		case PKTBI_D7::CUSTOM_HOUSE_ADD_MULTI:
			CustomHousesAddMulti(msg);
			break;
		
		case PKTBI_D7::CUSTOM_HOUSE_SYNCH:
			CustomHousesSynch(msg);
			break;

		case PKTBI_D7::CUSTOM_HOUSE_CLEAR:
			CustomHousesClear(msg);
			break;

		case PKTBI_D7::CUSTOM_HOUSE_SELECT_FLOOR:
			CustomHousesSelectFloor(msg);
			break;

		case PKTBI_D7::CUSTOM_HOUSE_REVERT:
			CustomHousesRevert(msg);
			break;
		case PKTBI_D7::GUILD_BUTTON:
			OnGuildButton(client);
			break;
		case PKTBI_D7::QUEST_BUTTON:
			OnQuestButton(client);
			break;
			//missing combat book abilities
		default:
			handle_unknown_packet(client);
	}
}
MESSAGE_HANDLER_VARLEN( PKTBI_D7, handle_aos_commands );

void OnGuildButton(Client* client)
{
	ref_ptr<EScriptProgram> prog = find_script( "misc/guildbutton", 
                                                true, 
                                                config.cache_interactive_scripts );
    if (prog.get() != NULL)
    {
        client->chr->start_script( prog.get(), false );
    }
}

void OnQuestButton(Client* client)
{
	ref_ptr<EScriptProgram> prog = find_script( "misc/questbutton", 
                                                true, 
                                                config.cache_interactive_scripts );
    if (prog.get() != NULL)
    {
        client->chr->start_script( prog.get(), false );
    }
}

void OnChatButton(Client* client)
{
	ref_ptr<EScriptProgram> prog = find_script( "misc/chatbutton", 
                                                true, 
                                                config.cache_interactive_scripts );
    if (prog.get() != NULL)
    {
        client->chr->start_script( prog.get(), false );
    }
}

