/** @file
 *
 * @par History
 */


#include <ctype.h>
#include <stddef.h>

#include "../bscript/berror.h"
#include "../bscript/impstr.h"
#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "item/item.h"
#include "mobile/charactr.h"
#include "module/unimod.h"
#include "module/uomod.h"
#include "network/cgdata.h"
#include "network/client.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "network/pktboth.h"
#include "ufunc.h"
#include "uoexec.h"
#include "uoexhelp.h"

namespace Pol
{
namespace Core
{
void send_prompt( Network::Client* client, u32 serial )
{
  Network::PktHelper::PacketOut<Network::PktOut_9A> msg;
  msg->WriteFlipped<u16>( sizeof msg->buffer );
  msg->Write<u32>( serial );
  msg->WriteFlipped<u32>( 0x15u );
  msg->offset += 5;  // u32 type u8 text[0]
  msg.Send( client );
}

void handle_prompt( Network::Client* client, PKTBI_9A* msg )
{
  Module::UOExecutorModule* uoemod = client->gd->prompt_uoemod;
  if ( uoemod == nullptr )
    return;
  int textlen = cfBEu16( msg->msglen ) - offsetof( PKTBI_9A, text );
  if ( msg->type )
  {
    if ( textlen <= 120 && msg->text[textlen - 1] == '\0' )
    {
      bool ok = true;
      --textlen;  // don't include null terminator (already checked)
      for ( int i = 0; i < textlen; ++i )
      {
        if ( !isprint( msg->text[i] ) )
        {
          ok = false;
          break;
        }
      }
      if ( ok )
      {
        Bscript::String* str = new Bscript::String( msg->text, textlen );
        uoemod->uoexec.ValueStack.back().set( new Bscript::BObject( str ) );
      }
    }
  }
  uoemod->uoexec.revive();
  uoemod->prompt_chr = nullptr;
  client->gd->prompt_uoemod = nullptr;
}
}  // namespace Core
namespace Module
{
Bscript::BObjectImp* UOExecutorModule::mf_RequestInput()
{
  Mobile::Character* chr;
  Items::Item* item;
  const Bscript::String* prompt;
  if ( !getCharacterParam( 0, chr ) || !getItemParam( 1, item ) ||
       !getStringParam( 2, prompt ) )
  {
    return new Bscript::BError( "Invalid parameter" );
  }

  if ( chr->client == nullptr )
  {
    return new Bscript::BError( "No client attached" );
  }

  if ( chr->has_active_prompt() != false )
  {
    return new Bscript::BError( "Another script has an active prompt" );
  }

  if ( !uoexec.suspend() )
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function UO::RequestInput():\n"
             << "\tThe execution of this script can't be blocked!\n";
    return new Bscript::BError( "Script can't be blocked" );
  }
  if ( !prompt->hasUTF8Characters() )
  {
    Core::send_sysmessage( chr->client, prompt->data() );

    chr->client->gd->prompt_uoemod = this;
    prompt_chr = chr;

    Core::send_prompt( chr->client, ctBEu32( item->serial ) );
  }
  else
  {
    Core::send_sysmessage_unicode( chr->client, prompt->value(), "ENU" );

    chr->client->gd->prompt_uoemod = this;
    prompt_chr = chr;

    Core::send_unicode_prompt( chr->client, ctBEu32( item->serial ) );
  }
  return new Bscript::BLong( 0 );
}
}  // namespace Module
}  // namespace Pol
