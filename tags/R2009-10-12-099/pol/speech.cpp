/*
History
=======

2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/08/01 MuadDib:   Rewrote where needed to do away with TEXTDEF struct useage. Pointless.
2009/08/25 Shinigami: STLport-5.2.1 fix: buffer not used
2009/09/03 MuadDib:   Relocation of account related cpp/h

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <stddef.h>
#include <ctype.h>
#include <wctype.h>

#include "../clib/endian.h"
#include "../clib/clib.h"
#include "../clib/logfile.h"
#include "../clib/random.h"
#include "../clib/strutil.h"

#include "accounts/account.h"
#include "network/client.h"
#include "listenpt.h"
#include "mkscrobj.h"
#include "msghandl.h"
#include "npc.h"
#include "pktin.h"
#include "pktout.h"
#include "polcfg.h"
#include "sockio.h"
#include "syshook.h"
#include "textcmd.h"
#include "tildecmd.h"
#include "uvars.h"
#include "ufunc.h"
#include "ufuncstd.h"

#include "../clib/fdump.h"
#include "../bscript/impstr.h"

//static char buffer[1024];

// ASCII-ONLY VERSIONS
void pc_spoke( NPC& npc, Character *chr, const char *text, int textlen, u8 texttype ) //DAVE
{
    npc.on_pc_spoke( chr, text, texttype );
}
void ghost_pc_spoke( NPC& npc, Character* chr, const char* text, int textlen, u8 texttype ) //DAVE
{
    npc.on_ghost_pc_spoke( chr, text, texttype );
}

// UNICODE VERSIONS
void pc_spoke( NPC& npc, Character *chr, const char *text, int textlen, u8 texttype, const u16 *wtext, const char lang[4], int wtextlen )
{
	npc.on_pc_spoke( chr, text, texttype, wtext, lang );
}
void ghost_pc_spoke( NPC& npc, Character* chr, const char *text, int textlen, u8 texttype, const u16 *wtext, const char lang[4], int wtextlen )
{
	npc.on_ghost_pc_spoke( chr, text, texttype, wtext, lang );
}

void handle_processed_speech( Client* client, char* textbuf, int textbuflen, char firstchar, u8 type, u16 color, u16 font )
{
	// ENHANCE: if (intextlen+1) != textbuflen, then the input line was 'dirty'.  May want to log this fact.
			
	if (textbuflen == 1)
		return;

	if (textbuf[0] == '.' || textbuf[0] == '=')
	{
		if (!process_command( client, textbuf ))
            send_sysmessage( client, string("Unknown command: ") + textbuf );
        return;
	}

    if (firstchar == '~') // we strip tildes out
    {
        process_tildecommand( client, textbuf );
        return;
    }
    
    if (client->chr->squelched())
        return;

    if (client->chr->hidden())
        client->chr->unhide();

    if (config.show_speech_colors)
    {
        cout << client->chr->name() << " speaking w/ color " 
             << hexint( cfBEu16(color) ) << endl;
    }

	// validate text color
	u16 textcol = cfBEu16(color);
	if ( textcol < 2 || textcol > 1001 )
	{
		textcol = 1001;
	}

	static PKTOUT_1C talkmsg;
    static PKTOUT_1C ghostmsg;
	unsigned short msglen = static_cast<unsigned short>(offsetof( PKTOUT_1C, text ) + textbuflen);
	talkmsg.msgtype         = PKTOUT_1C_ID;
	talkmsg.msglen          = ctBEu16( msglen );
	talkmsg.source_serial   = client->chr->serial_ext;
	talkmsg.source_graphic  = client->chr->graphic_ext;
	talkmsg.type    = type; // FIXME authorize
	talkmsg.color   = ctBEu16(textcol);
	talkmsg.font    = font;
	strzcpy( talkmsg.speaker_name, client->chr->name().c_str(), sizeof talkmsg.speaker_name );
	memcpy( talkmsg.text, textbuf, textbuflen );
	transmit( client, &talkmsg, msglen );

	if (client->chr->dead())
    {
        memcpy( &ghostmsg, &talkmsg, sizeof ghostmsg );
        char* t = ghostmsg.text;
        while (*t)
        {
            if (!isspace(*t))
            {
                if (random_int( 4 ) == 0)
                    *t = 'o';
                else
                    *t = 'O';
            }
            ++t;
        }
    }
	// send to those nearby
	for( unsigned cli = 0; cli < clients.size(); cli++ )
	{
		Client *client2 = clients[cli];
		if (!client2->ready) continue;
		if (client == client2) continue;
        if (!client2->chr->is_visible_to_me( client->chr ))
            continue;
		
        bool rangeok;
        if (type == TEXTTYPE_WHISPER)
            rangeok = in_whisper_range(  client->chr, client2->chr );
        else if (type == TEXTTYPE_YELL)
            rangeok = in_yell_range( client->chr, client2->chr );
        else
            rangeok = in_say_range( client->chr, client2->chr );
        if (rangeok)
        {
            if (!client->chr->dead() || 
                 client2->chr->dead() || 
                 client2->chr->can_hearghosts()  )
            {
    			transmit( client2, &talkmsg, msglen );
            }
            else
            {
                transmit( client2, &ghostmsg, msglen );
            }
        }
	}

    if (!client->chr->dead())
        for_nearby_npcs( pc_spoke, client->chr, textbuf, textbuflen, type );
    else
        for_nearby_npcs( ghost_pc_spoke, client->chr, textbuf, textbuflen, type );
    
    sayto_listening_points( client->chr, textbuf, textbuflen, type );
}

                              

void SpeechHandler( Client *client, PKTIN_03 *mymsg )
{
	int i;
	int intextlen;
	
	char textbuf[ SPEECH_MAX_LEN+1 ];
	int textbuflen;

	intextlen = cfBEu16(mymsg->msglen) - offsetof( PKTIN_03, text ) - 1;

	// Preprocess the text into a sanity-checked, printable, null-terminated form in textbuf
	if (intextlen < 0)
		intextlen = 0;
	if (intextlen > SPEECH_MAX_LEN) 
		intextlen = SPEECH_MAX_LEN;	// ENHANCE: May want to log this

	for( i = 0, textbuflen = 0; i < intextlen; i++ )
	{
		char ch = mymsg->text[i];

		if (ch == 0) break;
		if (ch == '~') continue;	// skip unprintable tildes.  Probably not a reportable offense.

		if (isprint( ch ))
			textbuf[ textbuflen++ ] = ch;
		// ENHANCE: else report client data error? Just log? 
	}
	textbuf[ textbuflen++ ] = 0;

    handle_processed_speech( client, textbuf, textbuflen, mymsg->text[0], mymsg->type, mymsg->color, mymsg->font );
}

MESSAGE_HANDLER_VARLEN( PKTIN_03, SpeechHandler );

void SendUnicodeSpeech(Client *client, PKTIN_AD *msgin, u16* wtext, size_t wtextlen, char* ntext, size_t ntextlen)
{
	using std::wstring;

	if (wtext[0] == ctBEu16(L'.') || wtext[0] == ctBEu16(L'='))
	{
		if (!process_command( client, ntext, wtext, msgin->lang ))
		{
			wstring wtmp(L"Unknown command: ");
			// Needs to be done char-by-char due to linux's 4-byte unicode!
			for(size_t i = 0; i < wtextlen; i++)
			wtmp += static_cast<wchar_t>(cfBEu16(wtext[i]));
			send_sysmessage( client, wtmp, msgin->lang );
		}
        return;
	}

    if (cfBEu16(msgin->wtext[0]) == L'~') // we strip tildes out
    {
        process_tildecommand( client, wtext );
        return;
    }

	if (client->chr->squelched())
		return;

    if (client->chr->hidden())
        client->chr->unhide();

    if (config.show_speech_colors)
    {
        cout << client->chr->name() << " speaking w/ color " 
             << hexint( cfBEu16(msgin->color) ) << endl;
    }

	// validate text color
	u16 textcol = cfBEu16(msgin->color);
	if ( textcol < 2 || textcol > 1001 )
	{
		// 3/8/2009 MuadDib Changed to default color instead of complain.
		textcol = 1001;
	}

	PKTOUT_AE ghostmsg;
	PKTOUT_AE talkmsg;
	unsigned short msglen = static_cast<unsigned short>(offsetof( PKTOUT_AE, wtext ) + wtextlen*sizeof(talkmsg.wtext[0]));
	talkmsg.msgtype         = PKTOUT_AE_ID;
	talkmsg.msglen          = ctBEu16( msglen );
	talkmsg.source_serial   = client->chr->serial_ext;
	talkmsg.source_graphic  = client->chr->graphic_ext;
	talkmsg.type    = msgin->type; // FIXME authorize
	talkmsg.color   = ctBEu16(textcol);
	talkmsg.font    = msgin->font;
	memcpy( talkmsg.lang, msgin->lang, sizeof talkmsg.lang );
	strzcpy( talkmsg.speaker_name, client->chr->name().c_str(), sizeof talkmsg.speaker_name );
	memcpy( talkmsg.wtext, wtext, wtextlen*sizeof(talkmsg.wtext[0]) );

	transmit( client, &talkmsg, msglen ); // self

	if (client->chr->dead())
	{
		unsigned short msglen = static_cast<unsigned short>(offsetof( PKTOUT_AE, wtext ) + wtextlen*sizeof(ghostmsg.wtext[0]));
		ghostmsg.msgtype         = PKTOUT_AE_ID;
		ghostmsg.msglen          = ctBEu16( msglen );
		ghostmsg.source_serial   = client->chr->serial_ext;
		ghostmsg.source_graphic  = client->chr->graphic_ext;
		ghostmsg.type    = msgin->type; // FIXME authorize
		ghostmsg.color   = ctBEu16(textcol);
		ghostmsg.font    = msgin->font;
		memcpy( ghostmsg.lang, msgin->lang, sizeof talkmsg.lang );
		strzcpy( ghostmsg.speaker_name, client->chr->name().c_str(), sizeof ghostmsg.speaker_name );
		memcpy( ghostmsg.wtext, wtext, wtextlen*sizeof(ghostmsg.wtext[0]) );

		u16* pwc = ghostmsg.wtext;
		while (*pwc != 0)
		{
            wchar_t wch = (*pwc);
            if (!iswspace(wch))
			{
                if (random_int( 4 ) == 0)
				{
                    *pwc = ctBEu16(L'o');
				}
				else
				{
                    *pwc = ctBEu16(L'O');
				}
			}
			++pwc;
		}
	}
		// send to those nearby
	for( unsigned cli = 0; cli < clients.size(); cli++ )
	{
		Client *client2 = clients[cli];
		if (!client2->ready) continue;
		if (client == client2) continue;
		if (!client2->chr->is_visible_to_me( client->chr )) 
			continue;

        bool rangeok;
        if (msgin->type == TEXTTYPE_WHISPER)
            rangeok = in_whisper_range(  client->chr, client2->chr ); //DAVE changed from hardcoded "2"
        else if (msgin->type == TEXTTYPE_YELL)
            rangeok = in_yell_range( client->chr, client2->chr ); //DAVE changed from hardcoded "25"
        else
            rangeok = in_say_range( client->chr, client2->chr ); //DAVE changed from "visual" range check, should be "say" range check.
        if (rangeok)
		{
            if (!client->chr->dead() || 
				client2->chr->dead() || 
				client2->chr->can_hearghosts()  )
			{
    			transmit( client2, &talkmsg, msglen );
			}
			else
			{
                transmit( client2, &ghostmsg, msglen );
			}
		}
	}

    if (!client->chr->dead())
		for_nearby_npcs( pc_spoke, client->chr, ntext, ntextlen, msgin->type,
						 wtext, msgin->lang, wtextlen);
	else
		for_nearby_npcs( ghost_pc_spoke, client->chr, ntext, ntextlen, msgin->type,
						 wtext, msgin->lang, wtextlen);

	sayto_listening_points( client->chr, ntext, ntextlen, msgin->type,
							wtext, msgin->lang, wtextlen);
}
u16 Get12BitNumber(u8 * thearray, u16 theindex)
{	
    u16 theresult = 0;
    int thenibble = theindex * 3;
    int thebyte = thenibble / 2;
    if (thenibble % 2)
        theresult = cfBEu16(*((u16 *) (thearray + thebyte))) & 0x0FFF;
    else
        theresult = cfBEu16(*((u16 *) (thearray + thebyte))) >> 4;
    return theresult;
}

int GetNextUTF8(u8 * bytemsg, int i,u16& unicodeChar)
{	u16 result = 0;

	if ((bytemsg[i] & 0x80) == 0)
	{
		unicodeChar = bytemsg[i];
		return i+1;
	}

	if ((bytemsg[i] & 0xE0) == 0xC0)
	{
		// two byte sequence :
		if ((bytemsg[i+1] & 0xC0) == 0x80)
		{
			result = ((bytemsg[i] & 0x1F) << 6) | (bytemsg[i+1] & 0x3F);
			unicodeChar = result;
			return i+2;
		}
	}
	else if ((bytemsg[i] & 0xF0) == 0xE0)
	{
		// three byte sequence
		if (((bytemsg[i+1] & 0xC0) == 0x80) &&
			((bytemsg[i+2] & 0xC0) == 0x80)
		   )
		{
			result = ((bytemsg[i] & 0x0F) << 12) | ((bytemsg[i+1] & 0x3F) < 6) | (bytemsg[i+2] & 0x3F);
			unicodeChar = result;
			return i+3;
		}
	}
	
	// An error occurred in the sequence(or sequence > 16 bits) :
	unicodeChar = 0x20;  // Set unicode char to a "space" character instead"
	return i+1;
}

void UnicodeSpeechHandler( Client *client, PKTIN_AD *msgin )
{
	using std::wcout; // wcout.narrow() function r0x! :-)

	int intextlen;
	u16 numtokens = 0;
	u16 * themsg = msgin->wtext;
	u8 *  bytemsg = ((u8 *) themsg);
	int wtextoffset = 0;
	ObjArray* speechtokens = NULL;
    BLong * atoken = NULL;
	int i;

	u16 tempbuf[ SPEECH_MAX_LEN+1 ];

    u16 wtextbuf[ SPEECH_MAX_LEN+1 ];
    size_t wtextbuflen;

	char ntextbuf[SPEECH_MAX_LEN+1];
	size_t ntextbuflen;

	if (msgin->type & 0xc0)
	{
		numtokens = Get12BitNumber((u8 *) (msgin->wtext), 0);
		wtextoffset = ((((numtokens+1)*3)/2) + ((numtokens+1) % 2));		bytemsg = (((u8*) themsg) + wtextoffset);
		int bytemsglen = cfBEu16(msgin->msglen) - wtextoffset - offsetof( PKTIN_AD, wtext ) - 1;
        intextlen = 0;

		i = 0;
		int j = 0;
        u16 unicodeChar;
		while ((i < bytemsglen) && (i < SPEECH_MAX_LEN))
		{
			i = GetNextUTF8(bytemsg, i, unicodeChar);
			tempbuf[j++] = cfBEu16(unicodeChar);
			intextlen++;
		}

		themsg = tempbuf;
	}
	else
		intextlen = (cfBEu16(msgin->msglen) - offsetof( PKTIN_AD, wtext ))
					/ sizeof(msgin->wtext[0]) - 1;

	// Preprocess the text into a sanity-checked, printable, null-terminated form in textbuf
	if (intextlen < 0)
		intextlen = 0;
	if (intextlen > SPEECH_MAX_LEN) 
		intextlen = SPEECH_MAX_LEN;	// ENHANCE: May want to log this

	// Preprocess the text into a sanity-checked, printable, null-terminated form
	// in 'wtextbuf' and 'ntextbuf'
	ntextbuflen = 0;
	wtextbuflen = 0;
    for( i = 0; i < intextlen; i++ )
    {
		u16 wc = cfBEu16(themsg[i]);
		if (wc == 0) break;		// quit early on embedded nulls
		if (wc == L'~') continue;	// skip unprintable tildes. 
		wtextbuf[ wtextbuflen++ ] = ctBEu16(wc);
		ntextbuf[ ntextbuflen++ ] = wcout.narrow((wchar_t)wc, '?');
	}
	wtextbuf[ wtextbuflen++ ] = (u16)0;
	ntextbuf[ ntextbuflen++ ] = 0;

	if (msgin->type & 0xc0)
	{
		if(system_hooks.speechmul_hook != NULL)
		{
			for (u16 i = 0; i < numtokens; i++)
			{
				if (speechtokens == NULL)
					speechtokens = new ObjArray();
				atoken = new BLong(Get12BitNumber((u8 *) (msgin->wtext), i+1));
				speechtokens->addElement(atoken);
			}
			system_hooks.speechmul_hook->call( make_mobileref(client->chr), speechtokens, new String(ntextbuf) );
		}
		msgin->type &= (~0xC0);  // Client won't accept C0 text type messages, so must set to 0
	}
	
	SendUnicodeSpeech(client, msgin, wtextbuf, wtextbuflen, ntextbuf, ntextbuflen);
}

MESSAGE_HANDLER_VARLEN( PKTIN_AD, UnicodeSpeechHandler );
