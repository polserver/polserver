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


#include <cctype>
#include <cstddef>
#include <iostream>
#include <string>

#include "../bscript/bobject.h"
#include "../bscript/impstr.h"
#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../clib/random.h"
#include "../clib/rawtypes.h"
#include "../plib/systemstate.h"
#include "globals/settings.h"
#include "globals/uvars.h"
#include "guilds.h"
#include "listenpt.h"
#include "mkscrobj.h"
#include "mobile/charactr.h"
#include "mobile/npc.h"
#include "network/client.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "network/pktdef.h"
#include "network/pktin.h"
#include "syshook.h"
#include "textcmd.h"
#include "tildecmd.h"
#include "ufunc.h"
#include "ufuncstd.h"
#include "uworld.h"

namespace Pol
{
namespace Core
{
void handle_processed_speech( Network::Client* client, const std::string& text, u8 type, u16 color,
                              u16 font )
{
  if ( text.empty() )
    return;

  std::string s_text( text );
  Clib::sanitizeUnicodeWithIso( &s_text );  // use original text for other clients
  Mobile::Character* chr = client->chr;

  // validate text color
  u16 textcol = cfBEu16( color );
  if ( textcol < 2 || textcol > 1001 )
  {
    textcol = 1001;
  }
  chr->last_textcolor( textcol );

  if ( s_text[0] == '.' || s_text[0] == '=' )
  {
    if ( !process_command( client, s_text ) )
      send_sysmessage( client, std::string( "Unknown command: " ) + text );
    return;
  }

  if ( s_text[0] == '~' )
  {
    process_tildecommand( client, s_text );
    return;
  }

  if ( chr->squelched() )
    return;

  if ( chr->hidden() )
    chr->unhide();

  if ( Plib::systemstate.config.show_speech_colors )
  {
    INFO_PRINT << chr->name() << " speaking w/ color 0x" << fmt::hexu( cfBEu16( color ) ) << "\n";
  }

  u16 textlen = static_cast<u16>( text.size() + 1 );
  if ( textlen > SPEECH_MAX_LEN + 1 )
    textlen = SPEECH_MAX_LEN + 1;

  Network::PktHelper::PacketOut<Network::PktOut_1C> talkmsg;
  talkmsg->offset += 2;
  talkmsg->Write<u32>( chr->serial_ext );
  talkmsg->WriteFlipped<u16>( chr->graphic );
  talkmsg->Write<u8>( type );  // FIXME authorize
  talkmsg->WriteFlipped<u16>( textcol );
  talkmsg->WriteFlipped<u16>( font );
  talkmsg->Write( chr->name().c_str(), 30 );
  talkmsg->Write( text.c_str(), textlen );
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
  if ( type == Plib::TEXTTYPE_WHISPER )
    range = Core::settingsManager.ssopt.whisper_range;
  else if ( type == Plib::TEXTTYPE_YELL )
    range = Core::settingsManager.ssopt.yell_range;
  else
    range = Core::settingsManager.ssopt.speech_range;
  Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
      chr->x(), chr->y(), chr->realm(), range,
      [&]( Mobile::Character* other_chr )
      {
        Network::Client* client2 = other_chr->client;
        if ( client == client2 )
          return;
        if ( !other_chr->is_visible_to_me( chr ) )
          return;
        if ( other_chr->deafened() )
          return;

        if ( !chr->dead() || other_chr->dead() || other_chr->can_hearghosts() ||
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
    Core::WorldIterator<Core::NPCFilter>::InRange( chr->x(), chr->y(), chr->realm(), range,
                                                   [&]( Mobile::Character* otherchr )
                                                   {
                                                     Mobile::NPC* npc =
                                                         static_cast<Mobile::NPC*>( otherchr );
                                                     npc->on_pc_spoke( chr, s_text, type );
                                                   } );
  }
  else
  {
    Core::WorldIterator<Core::NPCFilter>::InRange( chr->x(), chr->y(), chr->realm(), range,
                                                   [&]( Mobile::Character* otherchr )
                                                   {
                                                     Mobile::NPC* npc =
                                                         static_cast<Mobile::NPC*>( otherchr );
                                                     npc->on_ghost_pc_spoke( chr, s_text, type );
                                                   } );
  }

  sayto_listening_points( client->chr, s_text, type );
}


void SpeechHandler( Network::Client* client, PKTIN_03* mymsg )
{
  int intextlen = cfBEu16( mymsg->msglen ) - offsetof( PKTIN_03, text ) - 1;

  // Preprocess the text into a sanity-checked, printable form in text
  if ( intextlen < 0 )
    intextlen = 0;
  if ( intextlen > SPEECH_MAX_LEN )
    intextlen = SPEECH_MAX_LEN;  // ENHANCE: May want to log this

  std::string text;
  text.reserve( intextlen );
  for ( int i = 0; i < intextlen; i++ )
  {
    char ch = mymsg->text[i];

    if ( ch == 0 )
      break;
    if ( isprint( ch ) )
      text += ch;
    // ENHANCE: else report client data error? Just log?
  }
  handle_processed_speech( client, text, mymsg->type, mymsg->color, mymsg->font );
}

void SendUnicodeSpeech( Network::Client* client, PKTIN_AD* msgin, const std::string& text,
                        Bscript::ObjArray* speechtokens )
{
  if ( text.empty() )
    return;
  // validate text color
  u16 textcol = cfBEu16( msgin->color );
  if ( textcol < 2 || textcol > 1001 )
  {
    // 3/8/2009 MuadDib Changed to default color instead of complain.
    textcol = 1001;
  }

  Mobile::Character* chr = client->chr;

  chr->last_textcolor( textcol );

  if ( text[0] == '.' || text[0] == '=' )
  {
    std::string lang( msgin->lang );
    if ( !process_command( client, text, lang ) )
    {
      std::string tmp( "Unknown command: " + text );
      send_sysmessage_unicode( client, tmp, lang );
    }
    return;
  }

  if ( text[0] == '~' )
  {
    process_tildecommand( client, text );
    return;
  }

  if ( chr->squelched() )
    return;

  if ( chr->hidden() )
    chr->unhide();

  if ( Plib::systemstate.config.show_speech_colors )
  {
    INFO_PRINT << chr->name() << " speaking w/ color 0x" << fmt::hexu( cfBEu16( msgin->color ) )
               << "\n";
  }

  Network::PktHelper::PacketOut<Network::PktOut_AE> ghostmsg;
  Network::PktHelper::PacketOut<Network::PktOut_AE> talkmsg;
  talkmsg->offset += 2;
  talkmsg->Write<u32>( chr->serial_ext );
  talkmsg->WriteFlipped<u16>( chr->graphic );
  talkmsg->Write<u8>( msgin->type );  // FIXME authorize
  talkmsg->WriteFlipped<u16>( textcol );
  talkmsg->WriteFlipped<u16>( msgin->font );
  talkmsg->Write( msgin->lang, 4 );
  talkmsg->Write( chr->name().c_str(), 30 );

  std::vector<u16> utf16 = Bscript::String::toUTF16( text );
  if ( utf16.size() > SPEECH_MAX_LEN )
    utf16.resize( SPEECH_MAX_LEN );
  talkmsg->WriteFlipped( utf16, true );
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
        Network::Client* client2 = networkManager.clients[cli];
        if ( !client2->ready )
          continue;
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
        Network::Client* client2 = networkManager.clients[cli];
        if ( !client2->ready )
          continue;
        auto otherguild = client2->chr->guild();
        if ( otherguild != nullptr )
        {
          if ( thisguild->guildid() == otherguild->guildid() ||
               ( thisguild->hasAlly( otherguild ) ) )
            talkmsg.Send( client2, len );
        }
      }
    }
  }
  else
  {
    talkmsg.Send( client, len );  // self
    if ( chr->dead() && !chr->can_be_heard_as_ghost() )
    {
      memcpy( &ghostmsg->buffer, &talkmsg->buffer, sizeof ghostmsg->buffer );

      ghostmsg->offset = 48;
      u16* t = ( (u16*)&ghostmsg->buffer[ghostmsg->offset] );
      while ( ghostmsg->offset < len - 2 )  // dont convert nullterm
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
    if ( msgin->type == Plib::TEXTTYPE_WHISPER )
      range = Core::settingsManager.ssopt.whisper_range;
    else if ( msgin->type == Plib::TEXTTYPE_YELL )
      range = Core::settingsManager.ssopt.yell_range;
    else
      range = Core::settingsManager.ssopt.speech_range;
    Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
        chr->x(), chr->y(), chr->realm(), range,
        [&]( Mobile::Character* otherchr )
        {
          Network::Client* client2 = otherchr->client;
          if ( client == client2 )
            return;
          if ( !otherchr->is_visible_to_me( chr ) )
            return;
          if ( otherchr->deafened() )
            return;

          if ( !chr->dead() || otherchr->dead() || otherchr->can_hearghosts() ||
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
      Core::WorldIterator<Core::NPCFilter>::InRange(
          chr->x(), chr->y(), chr->realm(), range,
          [&]( Mobile::Character* otherchr )
          {
            Mobile::NPC* npc = static_cast<Mobile::NPC*>( otherchr );
            npc->on_pc_spoke( chr, text, msgin->type, msgin->lang, speechtokens );
          } );
    }
    else
    {
      Core::WorldIterator<Core::NPCFilter>::InRange(
          chr->x(), chr->y(), chr->realm(), range,
          [&]( Mobile::Character* otherchr )
          {
            Mobile::NPC* npc = static_cast<Mobile::NPC*>( otherchr );
            npc->on_ghost_pc_spoke( chr, text, msgin->type, msgin->lang, speechtokens );
          } );
    }
    sayto_listening_points( client->chr, text, msgin->type, msgin->lang, speechtokens );
  }
}
u16 Get12BitNumber( u8* thearray, u16 theindex )
{
  u16 theresult = 0;
  int thenibble = theindex * 3;
  int thebyte = thenibble / 2;
  if ( thenibble % 2 )
    theresult = cfBEu16( *( (u16*)( thearray + thebyte ) ) ) & 0x0FFF;
  else
    theresult = cfBEu16( *( (u16*)( thearray + thebyte ) ) ) >> 4;
  return theresult;
}

void UnicodeSpeechHandler( Network::Client* client, PKTIN_AD* msgin )
{
  u16 numtokens = 0;
  std::unique_ptr<Bscript::ObjArray> speechtokens( nullptr );
  std::string text;
  if ( msgin->type & 0xc0 )
  {
    numtokens = Get12BitNumber( (u8*)( msgin->wtext ), 0 );
    int wtextoffset = ( ( ( ( numtokens + 1 ) * 3 ) / 2 ) + ( ( numtokens + 1 ) % 2 ) );
    int bytemsglen = cfBEu16( msgin->msglen ) - wtextoffset - offsetof( Core::PKTIN_AD, wtext ) - 1;
    if ( bytemsglen < 0 )
      bytemsglen = 0;
    text = Bscript::String::fromUTF8( reinterpret_cast<const char*>( msgin->wtext ) + wtextoffset,
                                      bytemsglen );
  }
  else
  {
    int intextlen = ( cfBEu16( msgin->msglen ) - offsetof( Core::PKTIN_AD, wtext ) ) /
                        sizeof( msgin->wtext[0] ) -
                    1;
    if ( intextlen < 0 )
      intextlen = 0;
    text = Bscript::String::fromUTF16( msgin->wtext, intextlen, true );
  }
  //  SPEECH_MAX_LEN needs to be checked later

  if ( msgin->type & 0xc0 )
  {
    speechtokens.reset( new Bscript::ObjArray() );
    for ( u16 j = 0; j < numtokens; j++ )
    {
      speechtokens->addElement(
          new Bscript::BLong( Get12BitNumber( (u8*)( msgin->wtext ), j + 1 ) ) );
    }
    if ( gamestate.system_hooks.speechmul_hook )
    {
      gamestate.system_hooks.speechmul_hook->call( make_mobileref( client->chr ),
                                                   new Bscript::ObjArray( *speechtokens.get() ),
                                                   new Bscript::String( text ) );
    }
    msgin->type &= ( ~0xC0 );  // Client won't accept C0 text type messages, so must set to 0
  }

  SendUnicodeSpeech( client, msgin, text, speechtokens.release() );
}
}  // namespace Core
}  // namespace Pol
