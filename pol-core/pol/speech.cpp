/** @file
 *
 * @par History
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2009/08/01 MuadDib:   Rewrote where needed to do away with TEXTDEF struct useage. Pointless.
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: buffer not used
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 * - 2009/10/14 Turley:    new priv canbeheardasghost
 * - 2009/10/14 Turley:    Added char.deaf() methods & char.deafened member
 */


#include "accounts/account.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/msghandl.h"

#include "mobile/npc.h"

#include "listenpt.h"
#include "mkscrobj.h"
#include "pktin.h"
#include "pktout.h"
#include "polcfg.h"
#include "sockio.h"
#include "syshook.h"
#include "textcmd.h"
#include "tildecmd.h"
#include "globals/uvars.h"
#include "ufunc.h"
#include "ufuncstd.h"
#include "uworld.h"

#include "guilds.h"

#include "../bscript/impstr.h"

#include "../clib/clib_endian.h"
#include "../clib/clib.h"
#include "../clib/random.h"
#include "../clib/strutil.h"
#include "../clib/logfacility.h"
#include "../clib/fdump.h"

#include "../plib/systemstate.h"

#include <cstddef>
#include <cctype>
#include <cwctype>

#include <iomanip>
#include <iostream>

namespace Pol {
  namespace Core {

	void handle_processed_speech( Network::Client* client, char* textbuf, int textbuflen, char firstchar, u8 type, u16 color, u16 font )
	{
	  // ENHANCE: if (intextlen+1) != textbuflen, then the input line was 'dirty'.  May want to log this fact.

	  if ( textbuflen == 1 )
		return;

      Mobile::Character* chr = client->chr;

      // validate text color
      u16 textcol = cfBEu16( color );
      if ( textcol < 2 || textcol > 1001 )
      {
        textcol = 1001;
      }
      chr->last_textcolor( textcol );

	  if ( textbuf[0] == '.' || textbuf[0] == '=' )
	  {
		if ( !process_command( client, textbuf ) )
		  send_sysmessage( client, std::string( "Unknown command: " ) + textbuf );
		return;
	  }

	  if ( firstchar == '~' ) // we strip tildes out
	  {
		process_tildecommand( client, textbuf );
		return;
	  }

	  if ( chr->squelched() )
		return;

	  if ( chr->hidden() )
		chr->unhide();

	  if ( Plib::systemstate.config.show_speech_colors )
	  {
        INFO_PRINT << chr->name( ) << " speaking w/ color 0x"
		  << fmt::hexu( cfBEu16( color ) ) << "\n";
	  }

	  u16 textlen = static_cast<u16>( textbuflen + 1 );
	  if ( textlen > SPEECH_MAX_LEN + 1 )
		textlen = SPEECH_MAX_LEN + 1;

	  Network::PktHelper::PacketOut<Network::PktOut_1C> talkmsg;
	  talkmsg->offset += 2;
	  talkmsg->Write<u32>( chr->serial_ext );
	  talkmsg->WriteFlipped<u16>( chr->graphic );
	  talkmsg->Write<u8>( type ); // FIXME authorize
	  talkmsg->WriteFlipped<u16>( textcol );
	  talkmsg->WriteFlipped<u16>( font );
	  talkmsg->Write( chr->name().c_str(), 30 );
	  talkmsg->Write( textbuf, textlen );
	  u16 len = talkmsg->offset;
	  talkmsg->offset = 1;
	  talkmsg->WriteFlipped<u16>( len );
	  talkmsg.Send( client, len );

	  Network::PktHelper::PacketOut<Network::PktOut_1C> ghostmsg;
	  if ( chr->dead() && !chr->can_be_heard_as_ghost() )
	  {
		memcpy( &ghostmsg->buffer, &talkmsg->buffer, sizeof ghostmsg->buffer );
		ghostmsg->offset = 44;
		char* t = &ghostmsg->buffer[ghostmsg->offset];
		while ( ghostmsg->offset < len )
		{
		  if ( !isspace( *t ) )
		  {
            if ( Clib::random_int( 3 ) == 0 )
			  *t = 'o';
			else
			  *t = 'O';
		  }
		  ++t;
		  ghostmsg->offset++;
		}
	  }
	  // send to those nearby
      u16 range;
      if ( type == Core::TEXTTYPE_WHISPER )
        range = Core::settingsManager.ssopt.whisper_range;
      else if ( type == Core::TEXTTYPE_YELL )
        range = Core::settingsManager.ssopt.yell_range;
      else
        range = Core::settingsManager.ssopt.speech_range;
      Core::WorldIterator<Core::OnlinePlayerFilter>::InRange( chr->x, chr->y, chr->realm, range, [&]( Mobile::Character *other_chr )
      {
        Network::Client* client2 = other_chr->client;
        if ( client == client2 ) return;
        if ( !other_chr->is_visible_to_me( chr ) ) return;
        if ( other_chr->deafened( ) ) return;

        if ( !chr->dead() ||
             other_chr->dead() ||
             other_chr->can_hearghosts() ||
             chr->can_be_heard_as_ghost() )
        {
          talkmsg.Send( client2, len );
        }
        else
        {
          ghostmsg.Send( client2, len );
        }
      } );

	  if ( !chr->dead() )
      {
        Core::WorldIterator<Core::NPCFilter>::InRange( chr->x, chr->y, chr->realm, range, [&]( Mobile::Character *otherchr )
        {
          Mobile::NPC* npc = static_cast<Mobile::NPC*>( otherchr );
          npc->on_pc_spoke( chr, textbuf, type );
        } );
      }
      else
      {
        Core::WorldIterator<Core::NPCFilter>::InRange( chr->x, chr->y, chr->realm, range, [&]( Mobile::Character *otherchr )
        {
          Mobile::NPC* npc = static_cast<Mobile::NPC*>( otherchr );
          npc->on_ghost_pc_spoke( chr, textbuf, type );
        } );
      }

	  sayto_listening_points( client->chr, textbuf, textbuflen, type );
	}



	void SpeechHandler( Network::Client *client, PKTIN_03 *mymsg )
	{
	  int i;
	  int intextlen;

	  char textbuf[SPEECH_MAX_LEN + 1];
	  int textbuflen;

	  intextlen = cfBEu16( mymsg->msglen ) - offsetof( PKTIN_03, text ) - 1;

	  // Preprocess the text into a sanity-checked, printable, null-terminated form in textbuf
	  if ( intextlen < 0 )
		intextlen = 0;
	  if ( intextlen > SPEECH_MAX_LEN )
		intextlen = SPEECH_MAX_LEN;	// ENHANCE: May want to log this

	  for ( i = 0, textbuflen = 0; i < intextlen; i++ )
	  {
		char ch = mymsg->text[i];

		if ( ch == 0 ) break;
		if ( ch == '~' ) continue;	// skip unprintable tildes.  Probably not a reportable offense.

		if ( isprint( ch ) )
		  textbuf[textbuflen++] = ch;
		// ENHANCE: else report client data error? Just log? 
	  }
	  textbuf[textbuflen++] = 0;

	  handle_processed_speech( client, textbuf, textbuflen, mymsg->text[0], mymsg->type, mymsg->color, mymsg->font );
	}

    void SendUnicodeSpeech( Network::Client *client, PKTIN_AD *msgin, u16* wtext, size_t wtextlen, char* ntext, size_t ntextlen, Bscript::ObjArray* speechtokens )
	{
      // validate text color
      u16 textcol = cfBEu16( msgin->color );
      if ( textcol < 2 || textcol > 1001 )
      {
        // 3/8/2009 MuadDib Changed to default color instead of complain.
        textcol = 1001;
      }

      Mobile::Character* chr = client->chr;

      chr->last_textcolor( textcol );

	  using std::wstring;

	  if ( wtext[0] == ctBEu16( L'.' ) || wtext[0] == ctBEu16( L'=' ) )
	  {
		if ( !process_command( client, ntext, wtext, msgin->lang ) )
		{
		  wstring wtmp( L"Unknown command: " );
		  // Needs to be done char-by-char due to linux's 4-byte unicode!
		  for ( size_t i = 0; i < wtextlen; i++ )
			wtmp += static_cast<wchar_t>( cfBEu16( wtext[i] ) );
		  send_sysmessage( client, wtmp, msgin->lang );
		}
		return;
	  }

	  if ( cfBEu16( msgin->wtext[0] ) == L'~' ) // we strip tildes out
	  {
		process_tildecommand( client, wtext );
		return;
	  }

	  if ( chr->squelched() )
		return;

	  if ( chr->hidden() )
		chr->unhide();

	  if ( Plib::systemstate.config.show_speech_colors )
	  {
        INFO_PRINT << chr->name( ) << " speaking w/ color 0x"
		  << fmt::hexu( cfBEu16( msgin->color ) ) << "\n";
	  }

	  Network::PktHelper::PacketOut<Network::PktOut_AE> ghostmsg;
	  Network::PktHelper::PacketOut<Network::PktOut_AE> talkmsg;
	  talkmsg->offset += 2;
	  talkmsg->Write<u32>( chr->serial_ext );
	  talkmsg->WriteFlipped<u16>( chr->graphic );
	  talkmsg->Write<u8>( msgin->type ); // FIXME authorize
	  talkmsg->WriteFlipped<u16>( textcol );
	  talkmsg->WriteFlipped<u16>( msgin->font );
	  talkmsg->Write( msgin->lang, 4 );
	  talkmsg->Write( chr->name().c_str(), 30 );
	  talkmsg->Write( &wtext[0], static_cast<u16>( wtextlen ), false ); //nullterm already included
	  u16 len = talkmsg->offset;
	  talkmsg->offset = 1;
	  talkmsg->WriteFlipped<u16>( len );


	  if ( msgin->type == 0x0d )
	  {
        auto thisguild = chr->guild();
		if ( settingsManager.ssopt.core_sends_guildmsgs && thisguild != nullptr )
        {
		  for ( unsigned cli = 0; cli < networkManager.clients.size(); cli++ )
		  {
		    Network::Client *client2 = networkManager.clients[cli];
		    if ( !client2->ready ) continue;
		    if ( thisguild->guildid() == client2->chr->guildid() )
			  talkmsg.Send( client2, len );
		  }
        }
	  }
	  else if ( msgin->type == 0x0e )
	  {
        auto thisguild = chr->guild();
		if ( settingsManager.ssopt.core_sends_guildmsgs && thisguild != nullptr )
        {
		  for ( unsigned cli = 0; cli < networkManager.clients.size(); cli++ )
		  {
		    Network::Client *client2 = networkManager.clients[cli];
		    if ( !client2->ready ) continue;
            auto otherguild = client2->chr->guild();
            if (otherguild != nullptr)
            {
		      if ( thisguild->guildid() == otherguild->guildid() || ( thisguild->hasAlly( otherguild ) ) )
			    talkmsg.Send( client2, len );
            }
		  }
        }
	  }
	  else
	  {
		talkmsg.Send( client, len ); // self
		if ( chr->dead() && !chr->can_be_heard_as_ghost() )
		{
		  memcpy( &ghostmsg->buffer, &talkmsg->buffer, sizeof ghostmsg->buffer );

		  ghostmsg->offset = 48;
		  u16* t = ( (u16*)&ghostmsg->buffer[ghostmsg->offset] );
		  while ( ghostmsg->offset < len - 2 ) // dont convert nullterm
		  {
			wchar_t wch = ( *t );
			if ( !iswspace( wch ) )
			{
              if ( Clib::random_int( 3 ) == 0 )
				*t = ctBEu16( L'o' );
			  else
				*t = ctBEu16( L'O' );
			}
			++t;
			ghostmsg->offset += 2;
		  }
		}
		// send to those nearby
        u16 range;
        if ( msgin->type == Core::TEXTTYPE_WHISPER )
          range = Core::settingsManager.ssopt.whisper_range;
        else if ( msgin->type == Core::TEXTTYPE_YELL )
          range = Core::settingsManager.ssopt.yell_range;
        else
          range = Core::settingsManager.ssopt.speech_range;
        Core::WorldIterator<Core::OnlinePlayerFilter>::InRange( chr->x, chr->y, chr->realm, range, [&]( Mobile::Character *otherchr )
        {
          Network::Client* client2 = otherchr->client;
          if ( client == client2 ) return;
          if ( !otherchr->is_visible_to_me( chr ) ) return;
          if ( otherchr->deafened() ) return;

          if ( !chr->dead( ) ||
               otherchr->dead( ) ||
               otherchr->can_hearghosts( ) ||
               chr->can_be_heard_as_ghost( ) )
          {
            talkmsg.Send( client2, len );
          }
          else
          {
            ghostmsg.Send( client2, len );
          }
        } );

		if ( !chr->dead() )
        {
         Core::WorldIterator<Core::NPCFilter>::InRange( chr->x, chr->y, chr->realm, range, [&]( Mobile::Character *otherchr )
          {
            Mobile::NPC* npc = static_cast<Mobile::NPC*>( otherchr );
            npc->on_pc_spoke( chr, ntext, msgin->type, wtext, msgin->lang, speechtokens );
          } );
        }
        else
        {
          Core::WorldIterator<Core::NPCFilter>::InRange( chr->x, chr->y, chr->realm, range, [&]( Mobile::Character *otherchr )
          {
            Mobile::NPC* npc = static_cast<Mobile::NPC*>( otherchr );
            npc->on_ghost_pc_spoke( chr, ntext, msgin->type, wtext, msgin->lang, speechtokens );
          } );
        }
		sayto_listening_points( client->chr, ntext, static_cast<int>( ntextlen ), msgin->type,
								wtext, msgin->lang, static_cast<int>( wtextlen ), speechtokens );
	  }
	}
	u16 Get12BitNumber( u8 * thearray, u16 theindex )
	{
	  u16 theresult = 0;
	  int thenibble = theindex * 3;
	  int thebyte = thenibble / 2;
	  if ( thenibble % 2 )
		theresult = cfBEu16( *( (u16 *)( thearray + thebyte ) ) ) & 0x0FFF;
	  else
		theresult = cfBEu16( *( (u16 *)( thearray + thebyte ) ) ) >> 4;
	  return theresult;
	}

	int GetNextUTF8( u8 * bytemsg, int i, u16& unicodeChar )
	{
	  u16 result = 0;

	  if ( ( bytemsg[i] & 0x80 ) == 0 )
	  {
		unicodeChar = bytemsg[i];
		return i + 1;
	  }

	  if ( ( bytemsg[i] & 0xE0 ) == 0xC0 )
	  {
		// two byte sequence :
		if ( ( bytemsg[i + 1] & 0xC0 ) == 0x80 )
		{
		  result = ( ( bytemsg[i] & 0x1F ) << 6 ) | ( bytemsg[i + 1] & 0x3F );
		  unicodeChar = result;
		  return i + 2;
		}
	  }
	  else if ( ( bytemsg[i] & 0xF0 ) == 0xE0 )
	  {
		// three byte sequence
		if ( ( ( bytemsg[i + 1] & 0xC0 ) == 0x80 ) &&
			 ( ( bytemsg[i + 2] & 0xC0 ) == 0x80 )
			 )
		{
		  result = ( ( bytemsg[i] & 0x0F ) << 12 ) | ( ( bytemsg[i + 1] & 0x3F ) < 6 ) | ( bytemsg[i + 2] & 0x3F );
		  unicodeChar = result;
		  return i + 3;
		}
	  }

	  // An error occurred in the sequence(or sequence > 16 bits) :
	  unicodeChar = 0x20;  // Set unicode char to a "space" character instead"
	  return i + 1;
	}

	void UnicodeSpeechHandler( Network::Client *client, PKTIN_AD *msgin )
	{
	  using std::wcout; // wcout.narrow() function r0x! :-)

	  int intextlen;
	  u16 numtokens = 0;
	  u16 * themsg = msgin->wtext;
	  u8 *  bytemsg;
	  int wtextoffset = 0;
      std::unique_ptr<Bscript::ObjArray> speechtokens( nullptr );
	  int i;

	  u16 tempbuf[SPEECH_MAX_LEN + 1];

	  u16 wtextbuf[SPEECH_MAX_LEN + 1];
	  size_t wtextbuflen;

	  char ntextbuf[SPEECH_MAX_LEN + 1];
	  size_t ntextbuflen;

	  if ( msgin->type & 0xc0 )
	  {
		numtokens = Get12BitNumber( (u8 *)( msgin->wtext ), 0 );
		wtextoffset = ( ( ( ( numtokens + 1 ) * 3 ) / 2 ) + ( ( numtokens + 1 ) % 2 ) );
		bytemsg = ( ( (u8*)themsg ) + wtextoffset );
		int bytemsglen = cfBEu16( msgin->msglen ) - wtextoffset - offsetof( PKTIN_AD, wtext ) - 1;
		intextlen = 0;

		i = 0;
		int j = 0;
		u16 unicodeChar;
		while ( ( i < bytemsglen ) && ( i < SPEECH_MAX_LEN ) )
		{
		  i = GetNextUTF8( bytemsg, i, unicodeChar );
		  tempbuf[j++] = cfBEu16( unicodeChar );
		  intextlen++;
		}

		themsg = tempbuf;
	  }
	  else
		intextlen = ( cfBEu16( msgin->msglen ) - offsetof( PKTIN_AD, wtext ) )
		/ sizeof( msgin->wtext[0] ) - 1;

	  // Preprocess the text into a sanity-checked, printable, null-terminated form in textbuf
	  if ( intextlen < 0 )
		intextlen = 0;
	  if ( intextlen > SPEECH_MAX_LEN )
		intextlen = SPEECH_MAX_LEN;	// ENHANCE: May want to log this

	  // Preprocess the text into a sanity-checked, printable, null-terminated form
	  // in 'wtextbuf' and 'ntextbuf'
	  ntextbuflen = 0;
	  wtextbuflen = 0;
	  for ( i = 0; i < intextlen; i++ )
	  {
		u16 wc = cfBEu16( themsg[i] );
		if ( wc == 0 ) break;		// quit early on embedded nulls
		if ( wc == L'~' ) continue;	// skip unprintable tildes. 
		wtextbuf[wtextbuflen++] = ctBEu16( wc );
		ntextbuf[ntextbuflen++] = wcout.narrow( (wchar_t)wc, '?' );
	  }
	  wtextbuf[wtextbuflen++] = (u16)0;
	  ntextbuf[ntextbuflen++] = 0;

	  if ( msgin->type & 0xc0 )
	  {
        Bscript::BLong * atoken = NULL;
		if ( speechtokens.get() == nullptr )
          speechtokens.reset( new Bscript::ObjArray( ) );
		for ( u16 j = 0; j < numtokens; j++ )
		{
          atoken = new Bscript::BLong( Get12BitNumber( (u8 *)( msgin->wtext ), j + 1 ) );
		  speechtokens->addElement( atoken );
		}
		if ( gamestate.system_hooks.speechmul_hook != NULL )
		{
          gamestate.system_hooks.speechmul_hook->call( make_mobileref( client->chr ), new Bscript::ObjArray( *speechtokens.get( ) ), new Bscript::String( ntextbuf ) );
		}
		msgin->type &= ( ~0xC0 );  // Client won't accept C0 text type messages, so must set to 0
	  }

	  SendUnicodeSpeech( client, msgin, wtextbuf, wtextbuflen, ntextbuf, ntextbuflen, speechtokens.release() );
	}
  }
}