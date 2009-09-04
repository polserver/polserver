/*
History
=======
2005/01/24 Shinigami: added message handler for packet 0xd9 (Spy on Client 2)
2005/01/27 Shinigami: using little/big endian functions provided by endian.h
2005/04/03 Shinigami: send_feature_enable() call moved from start_client_char()
                      to send_start() to send before char selection
2005/04/03 Shinigami: FLAG_UPTO_SIX_CHARACTERS disabled in uo_feature_enable
2005/04/04 Shinigami: added candelete script
2005/08/29 Shinigami: get-/setspyonclient2 renamed to get-/setclientinfo
2007/07/09 Shinigami: modified PKTOUT_8C.new_key to not conflict with UO:KR detection
                      added message handler for packet 0xe4 (UO:KR Verifier Response)
2008/02/09 Shinigami: removed hardcoded MAX_CHARS from send_start()
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/08/06 MuadDib:   Removed PasswordOnlyHash support

Notes
=======

*/

#include "clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include "clib/stlutil.h"
#include "clib/MD5.h"

#include <string.h>


#include "clib/clib.h"
#include "clib/endian.h"
#include "clib/logfile.h"
#include "clib/fdump.h"

#include "accounts/account.h"
#include "charactr.h"
#include "client.h"
#include "core.h"
#include "msghandl.h"
#include "pktin.h"
#include "polcfg.h"
#include "startloc.h"
#include "uvars.h"
#include "servdesc.h"
#include "sockio.h"
#include "ssopt.h"
#include "ufunc.h"

bool is_banned_ip(Client* client);

void send_login_error( Client *client, unsigned char reason )
{
	PKTOUT_82 msg;
	msg.msgtype = PKTOUT_82_ID;
	msg.error = reason;
	client->transmit( &msg, sizeof msg );
}

bool acct_check(Client* client, int i)
{
	if (servers[i]->acct_match.empty())
		return true;

	for ( unsigned j = 0; j < servers[i]->acct_match.size(); ++j )
	{
		if (stricmp(servers[i]->acct_match[j].c_str(), client->acct->name()) == 0)
			return true;
	}

	return false;
}

bool server_applies( Client* client, int i )
{
    if (servers[i]->ip_match.empty())
        return acct_check(client, i);

    for( unsigned j = 0; j < servers[i]->ip_match.size(); ++j )
    {
        unsigned long addr1part, addr2part;
        struct sockaddr_in* sockin = reinterpret_cast<struct sockaddr_in*>(&client->ipaddr);

        addr1part = servers[i]->ip_match[j]      & servers[i]->ip_match_mask[j];
#ifdef _WIN32
        addr2part = sockin->sin_addr.S_un.S_addr & servers[i]->ip_match_mask[j];
#else
        addr2part = sockin->sin_addr.s_addr      & servers[i]->ip_match_mask[j];
#endif
        if (addr1part == addr2part)
            return true;
    }
    return false;
}

void loginserver_login( Client *client, PKTIN_80 *msg )
{
	unsigned idx;

    if (is_banned_ip( client ))
    {
        send_login_error( client, LOGIN_ERROR_ACCOUNT_BLOCKED );
        client->disconnect = 1;
        return;
    }

	Account *acct = find_account( msg->name );
	if (!acct)
	{
		send_login_error( client, LOGIN_ERROR_NO_ACCOUNT );
		client->disconnect = 1;
		return;
	}

	bool correct_password = false;

	string msgpass = msg->password;
	string acctname = acct->name();
	string temp;
	MD5_Encrypt(acctname + msgpass,temp); //MD5
	correct_password = MD5_Compare(acct->passwordhash(), temp);

	if (!correct_password)
	{
		send_login_error( client, LOGIN_ERROR_WRONG_PASSWORD );
		client->disconnect = 1;
        Log( "Incorrect password for account %s from %s\n",
             acct->name(),
             AddressToString( &client->ipaddr ) );
		return;
	}
	else
	{
		if(config.retain_cleartext_passwords)
		{
			if(acct->password().empty())
				acct->set_password(msgpass);
		}
	}

    if (!acct->enabled() || acct->banned())
    {
        send_login_error( client, LOGIN_ERROR_ACCOUNT_BLOCKED );
        client->disconnect = 1;
        return;
    }

    Log( "Account %s logged in from %s\n",
         acct->name(),
         AddressToString( &client->ipaddr ));

	client->acct = acct;

	static char buffer[ 2000 ];
	PKTOUT_A8 *phead = reinterpret_cast<PKTOUT_A8*>(buffer);
	PKTOUT_A8_SERVER *pelem = reinterpret_cast<PKTOUT_A8_SERVER*>(phead+1);
    unsigned short msglen = sizeof *phead;
    unsigned short servcount = 0;

	phead->msgtype = PKTOUT_A8_ID;
	phead->msglen = 0;
	phead->unk3_FF = 0xFF;
	phead->servcount = 0;

    for( idx = 0; idx < servers.size(); idx++ )
	{
        ServerDescription* server = servers[idx];

        if (!server->hostname.empty())
        {
            struct hostent* he = gethostbyname( server->hostname.c_str() ); // FIXME: here is a potential server lockup
            if (he && he->h_addr_list[0])
            {
                char* addr = he->h_addr_list[0];
                server->ip[0] = addr[3];
                server->ip[1] = addr[2];
                server->ip[2] = addr[1];
                server->ip[3] = addr[0];
            }
            else
            {
                Log( "gethostbyname(\"%s\") failed for server %s\n",
                      server->hostname.c_str(),
                      server->name.c_str() );
                continue;
            }
        }

        if (server_applies( client, idx ))
        {
            memset( pelem, 0, sizeof *pelem );
            ++servcount;
            pelem->servernum = ctBEu16( idx+1 );
		    strzcpy( pelem->servername, server->name.c_str(), sizeof pelem->servername );
		    pelem->servernum2 = ctBEu16( idx+1 );
            pelem->ip[0] = server->ip[3];
		    pelem->ip[1] = server->ip[2];
		    pelem->ip[2] = server->ip[1];
		    pelem->ip[3] = server->ip[0];
            ++pelem;
            msglen += sizeof *pelem;
        }
	}
	phead->msglen = ctBEu16( msglen );
	phead->servcount = ctBEu16( servcount );
	client->transmit( buffer, msglen );

    if (servcount == 0)
    {
        Log( "No applicable servers for client connecting from %s\n", AddressToString( &client->ipaddr ) );
    }
}
MESSAGE_HANDLER( PKTIN_80, loginserver_login );

void handle_A4( Client *client, PKTIN_A4 *msg )
{
}
MESSAGE_HANDLER( PKTIN_A4, handle_A4 );

void handle_D9( Client *client, PKTIN_D9 *msg )
{
  // Transform Little-Endian <-> Big-Endian
  msg->instance =       cfBEu32(msg->instance);       // Unique Instance ID of UO
  msg->os_major =       cfBEu32(msg->os_major);       // OS Major
  msg->os_minor =       cfBEu32(msg->os_minor);       // OS Minor
  msg->os_revision =    cfBEu32(msg->os_revision);    // OS Revision
  msg->cpu_family =     cfBEu32(msg->cpu_family);     // CPU Family
  msg->cpu_model =      cfBEu32(msg->cpu_model);      // CPU Model
  msg->cpu_clockspeed = cfBEu32(msg->cpu_clockspeed); // CPU Clock Speed [Mhz]
  msg->memory =         cfBEu32(msg->memory);         // Memory [MB]
  msg->screen_width =   cfBEu32(msg->screen_width);   // Screen Width
  msg->screen_height =  cfBEu32(msg->screen_height);  // Screen Height
  msg->screen_depth =   cfBEu32(msg->screen_depth);   // Screen Depth [Bit]
  msg->directx_major =  cfBEu16(msg->directx_major);  // DirectX Major
  msg->directx_minor =  cfBEu16(msg->directx_minor);  // DirectX Minor

  for ( unsigned i = 0; i < sizeof(msg->video_description) / sizeof(msg->video_description[0]); ++i )
    msg->video_description[i] = cfBEu16(msg->video_description[i]); // Video Card Description [wide-character]

  msg->video_vendor =   cfBEu32(msg->video_vendor);   // Video Card Vendor ID
  msg->video_device =   cfBEu32(msg->video_device);   // Video Card Device ID
  msg->video_memory =   cfBEu32(msg->video_memory);   // Video Card Memory [MB]

  for ( unsigned i = 0; i < sizeof(msg->langcode) / sizeof(msg->langcode[0]); ++i )
    msg->langcode[i] = cfBEu16(msg->langcode[i]); // Language Code [wide-character]

  client->setclientinfo(msg);
}
MESSAGE_HANDLER( PKTIN_D9, handle_D9 );

void select_server(Client *client, PKTIN_A0 *msg ) // Relay player to a certain IP
{
	unsigned servernum = cfBEu16(msg->servernum) - 1;

	if (servernum >= servers.size())
	{
		client->disconnect = 1;
		return;
	}

	ServerDescription *svr = servers[ servernum ];

	PKTOUT_8C rsp;
	rsp.msgtype = PKTOUT_8C_ID;
	rsp.ip[0] = svr->ip[3];
	rsp.ip[1] = svr->ip[2];
	rsp.ip[2] = svr->ip[1];
	rsp.ip[3] = svr->ip[0];
	if (client->listen_port != 0)
        rsp.port = ctBEu16( client->listen_port );
    else
        rsp.port = ctBEu16( svr->port );
	// MuadDib Added new seed system. This is for transferring KR/6017/Normal client detection from loginserver
	// to the gameserver. Allows keeping client flags from remote loginserver to gameserver for 6017 and kr
	// packets.
	// FIXME : in 099 Rewrite, upgrade this to some sort of bitflag setup?
	rsp.unk7_00 = 0xFE; // This was set to 0xffffffff in the past but this will conflict with UO:KR detection
	rsp.unk8_03 = 0xFE;
	if ( client->compareVersion(CLIENT_VER_60142))
		rsp.unk9_C3 = 0xFD;
	else
		rsp.unk9_C3 = 0xFE;
	// 0xFE = Normal
	// 0xFD = 6017+ client
	// 0xFC = KR client
	// FUCK EA :(
	if ( client->isUOKR )
		rsp.unk10_4B =0xFC;
	else if ( client->is_greq_6017 )
		rsp.unk10_4B =0xFD;
	else
		rsp.unk10_4B =0xFE;

	client->transmit( &rsp, sizeof rsp );

    unsigned long nseed = 0xFFffFFffLu; // CRYPT_AUTOMASK;
    client->clicrypt.setseed( client->cryptseed );
    client->cryptengine->Init( &nseed /*client->cryptseed*/ );
}

MESSAGE_HANDLER( PKTIN_A0, select_server );

void send_start( Client *client )
{
  send_feature_enable( client ); // Shinigami: moved from start_client_char() to send before char selection

	unsigned i;
    unsigned msglen;
	u32 clientflag; // sets client flags
	unsigned short char_slots; // number of slots according to expansion, avoids crashing people
	unsigned short char_count; // number of chars to send: Max(char_slots, 5)

	PKTOUT_A9 head;
	PKTOUT_A9_CHARACTERS char_elem;
	PKTOUT_A9_START_LEN startcount;
	PKTOUT_A9_START_ELEM startelem;
	PKTOUT_A9_START_FLAGS startflags;

	char_slots = config.character_slots; // sets it first to be the number defined in the config
	// TODO: Per account character slots? (With the actual character_slots defining maximum)

	// If more than 6 chars and no AOS, only send 5. Client is so boring sometimes...
	if (char_slots >= 6 && !(client->UOExpansionFlag & AOS)) {
		char_slots = 5;
	}

	char_count = 5; // UO always expects a minimum of 5? What a kludge...
	if (char_slots > char_count) // Max(char_slots, 5)
		char_count = char_slots;

	msglen = sizeof head +
             sizeof char_elem * char_count +
             sizeof startcount +
             sizeof startelem * startlocations.size() +
			 sizeof startflags ;

    char* msg = new char[msglen];
    unsigned int next_offset = 0;

    head.msgtype = PKTOUT_A9_ID;
    head.msglen = ctBEu16(msglen);
	head.numchars = static_cast<u8>(char_count); // Hope this works...

    memcpy(msg+next_offset,&head,sizeof head);
    next_offset += sizeof head;

	for( i = 0; i < char_count; i++ )
	{
        memset( &char_elem, 0, sizeof char_elem );
		if (i < char_slots) { // Small kludge to have a minimum of 5 chars in the packet
			Character* chr = client->acct->get_character( i );
			if (chr)
			{
				strzcpy( char_elem.name,
						 chr->name().c_str(),
						 sizeof char_elem.name );
			}
		}
		//client->transmitmore( &char_elem, sizeof char_elem );
        memcpy(msg+next_offset,&char_elem,sizeof char_elem);
        next_offset += sizeof char_elem;
	}

	startcount.startcount = (u8) startlocations.size();
	//client->transmitmore( &startcount, sizeof startcount );
    memcpy(msg+next_offset,&startcount,sizeof startcount);
    next_offset += sizeof startcount;

	for( i = 0; i < startlocations.size(); i++ )
	{
        memset( &startelem, 0, sizeof startelem );
		startelem.startnum = static_cast<u8>(i);
		strzcpy( startelem.city, startlocations[i]->city.c_str(), sizeof startelem.city );
		strzcpy( startelem.desc, startlocations[i]->desc.c_str(), sizeof startelem.desc );
		//client->transmitmore( &startelem, sizeof startelem );
        memcpy(msg+next_offset,&startelem,sizeof startelem);
        next_offset += sizeof startelem;
	}


	clientflag = ssopt.uo_feature_enable; // 'default' flags. Maybe auto-enable them according to the expansion?

	// Change this to a function for clarity? -- Nando
	if (char_slots == 7) {
		clientflag |= 0x1000; // 7th Character flag
	}
	else if (char_slots == 6) {
		clientflag |= 0x40; // 6th Character Flag
	}
	else if (char_slots == 1) {
		clientflag |= 0x14; // Only one character (SIEGE (0x04) + LIMIT_CHAR (0x10))		
	}
  
	startflags.flags = ctBEu32(clientflag);

    memcpy(msg+next_offset,&startflags,sizeof startflags);
	//client->transmitmore( &startflags, sizeof startflags );
    client->transmit( msg, msglen );
    delete[] msg;
}




void login2(Client *client, PKTIN_91 *msg) // Gameserver login and character listing
{
	client->encrypt_server_stream = 1;

    if (is_banned_ip( client ))
    {
        send_login_error( client, LOGIN_ERROR_ACCOUNT_BLOCKED );
        client->disconnect = 1;
        return;
    }

	/* Hmm, might have to re-search for account.
	   For now, we already have the account in client->acct.
	   Might work different if real loginservers were used. */
	Account *acct = find_account( msg->name );
	if (acct == NULL)
	{
		send_login_error( client, LOGIN_ERROR_NO_ACCOUNT );
		client->disconnect = 1;
		return;
	}

	// First check the password - if wrong, you can't find out anything else.
	bool correct_password = false;

	//dave changed 6/5/3, always authenticate with hashed user+pass
	string msgpass = msg->password;
	string acctname = acct->name();
	string temp;
	MD5_Encrypt(acctname + msgpass,temp); //MD5
	correct_password = MD5_Compare(acct->passwordhash(), temp);

	if (!correct_password)
	{
		send_login_error( client, LOGIN_ERROR_WRONG_PASSWORD );
		client->disconnect = 1;
        Log( "Incorrect password for account %s from %s\n",
             acct->name(),
             AddressToString( &client->ipaddr ) );
		return;
	}
	else
	{
		//write out cleartext if necessary
		if(config.retain_cleartext_passwords)
		{
			if(acct->password().empty())
				acct->set_password(msgpass);
		}
	}

    if (!acct->enabled() || acct->banned())
    {
        send_login_error( client, LOGIN_ERROR_ACCOUNT_BLOCKED );
        client->disconnect = 1;
        return;
    }

	//
	//Dave moved the max_clients check to pol.cpp so character cmdlevel could be checked.
	//

    Log( "Account %s logged in from %s\n",
         acct->name(),
         AddressToString( &client->ipaddr ));

	// ENHANCEMENT: could authenticate with real loginservers.

	client->acct = acct;
	/* NOTE: acct->client is not set here.  It is possible that another client
	   is still connected, or a connection is stuck open, or similar.  When
	   a character is selected, if another client is connected, measures will
	   be taken. */

	// Tell the client about the starting locations and his characters (up to 5).

	// MuadDib Added new seed system. This is for transferring KR/6017/Normal client detection from loginserver
	// to the gameserver. Allows keeping client flags from remote loginserver to gameserver for 6017 and kr
	// packets.
	// FIXME : in 099 Rewrite, upgrade this to some sort of bitflag setup?
	// 0xFE = Normal
	// 0xFD = 6017+ client
	// 0xFC = KR client
	// FUCK EA :(
	if (msg->unk3==0xFD)
		client->is_greq_60142=true;

	switch ( msg->unk4 )
	{
		case 0xFC: client->isUOKR = true; break;
		case 0xFD: client->is_greq_6017 = true; break;
//	case 0xFE: 
//	default:
	}

	send_start( client );
}

MESSAGE_HANDLER( PKTIN_91, login2 );

void delete_character( Account* acct, Character* chr, int charidx )
{
    if (!chr->logged_in)
    {
        Log( "Account %s deleting character 0x%lu\n", acct->name(), chr->serial );

        chr->acct.clear();
        acct->clear_character( charidx );

        chr->destroy();
    }
}

bool can_delete_character( Character* chr, int delete_by );
void call_ondelete_scripts( Character* chr );

void handle_delete_character( Client* client, PKTIN_83* msg )
{
    u32 charidx = cfBEu32( msg->charidx );

	if ((charidx < 0) || (charidx >= config.character_slots) ||
		(client->acct == NULL) ||
        (client->acct->get_character( charidx ) == NULL))
	{
		send_login_error( client, LOGIN_ERROR_MISC );
		client->disconnect = 1;
		return;
	}

    Account* acct = client->acct;
    Character* chr = acct->get_character( charidx );
	if (chr->client != NULL ||
        acct->active_character != NULL) // this account has a currently active character.
	{
		send_login_error( client, LOGIN_ERROR_OTHER_CHAR_INUSE );
		client->disconnect = 1;
		return;
    }
    
    if (can_delete_character( chr, DELETE_BY_PLAYER ))
    {
        call_ondelete_scripts( chr );
        delete_character( acct, chr, charidx );
    }
    
    send_start( client );
}
MESSAGE_HANDLER( PKTIN_83, handle_delete_character );

void KR_Verifier_Response( Client *client, PKTIN_E4 *msg )
{
	//
}
MESSAGE_HANDLER_VARLEN( PKTIN_E4, KR_Verifier_Response );
