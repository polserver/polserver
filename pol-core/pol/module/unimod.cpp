/** @file
 *
 * @par History
 * - 2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 */

#include "unimod.h"
#include <iostream>
#include <string.h>

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/impstr.h"
#include "../../clib/clib_endian.h"
#include "../../clib/logfacility.h"
#include "../accounts/account.h"
#include "../item/item.h"
#include "../mobile/charactr.h"
#include "../network/cgdata.h"
#include "../network/client.h"
#include "../network/packethelper.h"
#include "../network/packets.h"
#include "../network/pktboth.h"
#include "../network/pktdef.h"
#include "../ufunc.h"
#include "../uoexec.h"
#include "uomod.h"

#include <module_defs/unicode.h>

namespace Pol
{
namespace Core
{
using namespace Network;

void send_unicode_prompt( Client* client, u32 serial )
{
  PktHelper::PacketOut<PktOut_C2> msg;
  msg->WriteFlipped<u16>( Core::PKTBI_C2::SERVER_MSGLEN );
  msg->Write<u32>( serial );  // serial
  msg->Write<u32>( serial );  // msg_id  Server-"decided" message ID. o_O
  msg->offset += 10;          // 10x u8 unk
  msg.Send( client );
}

void handle_unicode_prompt( Client* client, Core::PKTBI_C2* msg )
{
  // TODO do we really need two of them, this can also be done differently
  Module::UnicodeExecutorModule* uniemod = client->gd->prompt_uniemod;
  Module::UOExecutorModule* uoemod = client->gd->prompt_uoemod;
  if ( uoemod == nullptr && uniemod == nullptr )
    return;  // log it?

  int textlen = ( ( cfBEu16( msg->msglen ) - offsetof( Core::PKTBI_C2, wtext ) ) /
                  sizeof( msg->wtext[0] ) );  // note NO terminator!
  if ( textlen < 0 )
    textlen = 0;
  if ( textlen > SPEECH_MAX_LEN )
  {
    textlen = SPEECH_MAX_LEN;  // ENHANCE: May want to log this
    msg->wtext[textlen] = 0x0000;
  }

  // client version of the packet should always send this as a 1??
  /*if ( cfBEu16( msg->unk ) != 0x01 )
  {
    // ENHANCE: May want to log this, too?
    ok = false;
    valstack = new BObject( new BError( "Malformed return-packet from client" ) );
  }*/

  char lang[4];
  memcpy( lang, msg->lang, 4 );

  // String::fromUTF16 performs all the error checks
  // "Invalid unicode" will be returned if completly broken, but do we really need to take care?
  if ( uniemod != nullptr && uniemod->prompt_chr != nullptr )
  {
    std::unique_ptr<Bscript::BStruct> retval( new Bscript::BStruct() );
    retval->addMember( "lang", new Bscript::String( lang ) );
    retval->addMember( "text",
                       new Bscript::String( Bscript::String::fromUTF16( msg->wtext, textlen ) ) );
    uniemod->exec.ValueStack.back().set( new Bscript::BObject( retval.release() ) );
    uniemod->uoexec.revive();
  }
  else if ( uoemod != nullptr && uoemod->prompt_chr != nullptr )
  {
    // called from uo module, directly return string
    uoemod->exec.ValueStack.back().set( new Bscript::BObject(
        new Bscript::String( Bscript::String::fromUTF16( msg->wtext, textlen ) ) ) );

    uoemod->uoexec.revive();
  }
  if ( uniemod != nullptr )
    uniemod->prompt_chr = nullptr;
  if ( uoemod != nullptr )
    uoemod->prompt_chr = nullptr;
  client->gd->prompt_uniemod = nullptr;
  client->gd->prompt_uoemod = nullptr;
}
//////////////////////////////////////////////////////////////////////////
}  // namespace Core
namespace Module
{
using namespace Bscript;

UnicodeExecutorModule::UnicodeExecutorModule( Core::UOExecutor& exec )
    : TmplExecutorModule<UnicodeExecutorModule, Core::PolModule>( exec ),
      uoexec( exec ),
      prompt_chr( nullptr )
{
}

UnicodeExecutorModule::~UnicodeExecutorModule()
{
  if ( prompt_chr != nullptr )
  {
    prompt_chr->client->gd->prompt_uniemod = nullptr;
    prompt_chr = nullptr;
  }
}

BObjectImp* UnicodeExecutorModule::mf_BroadcastUC()
{
  const String* text;
  const String* lang;
  unsigned short font;
  unsigned short color;
  unsigned short requiredCmdLevel;
  if ( getUnicodeStringParam( 0, text ) && getStringParam( 1, lang ) &&
       getParam( 2, font ) &&             // todo: getFontParam
       getParam( 3, color ) &&            // todo: getColorParam
       getParam( 4, requiredCmdLevel ) )  // todo: getRequiredCmdLevelParam
  {
    if ( text->length() > SPEECH_MAX_LEN )
      return new BError( "Text exceeds maximum size." );
    if ( lang->length() != 3 )
      return new BError( "langcode must be a 3-character code." );
    Core::broadcast_unicode( text->value(), Clib::strupperASCII( lang->value() ), font, color,
                             requiredCmdLevel );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "A parameter was invalid" );
  }
}

BObjectImp* UnicodeExecutorModule::mf_PrintTextAboveUC()
{
  Core::UObject* obj;
  const String* text;
  const String* lang;
  unsigned short font;
  unsigned short color;
  int journal_print;

  if ( getUObjectParam( 0, obj ) && getUnicodeStringParam( 1, text ) && getStringParam( 2, lang ) &&
       getParam( 3, font ) && getParam( 4, color ) && getParam( 5, journal_print ) )
  {
    if ( text->length() > SPEECH_MAX_LEN )
      return new BError( "Text exceeds maximum size." );
    if ( lang->length() != 3 )
      return new BError( "langcode must be a 3-character code." );

    return new BLong( say_above_unicode( obj, text->value(), Clib::strupperASCII( lang->value() ),
                                         font, color, journal_print ) );
  }
  else
  {
    return new BError( "A parameter was invalid" );
  }
}

BObjectImp* UnicodeExecutorModule::mf_PrintTextAbovePrivateUC()
{
  Mobile::Character* chr;
  Core::UObject* obj;
  const String* text;
  const String* lang;
  unsigned short font;
  unsigned short color;

  if ( getUObjectParam( 0, obj ) && getUnicodeStringParam( 1, text ) && getStringParam( 2, lang ) &&
       getCharacterParam( 3, chr ) && getParam( 4, font ) && getParam( 5, color ) )
  {
    if ( text->length() > SPEECH_MAX_LEN )
      return new BError( "Text exceeds maximum size." );
    if ( lang->length() != 3 )
      return new BError( "langcode must be a 3-character code." );

    return new BLong( private_say_above_unicode(
        chr, obj, text->value(), Clib::strupperASCII( lang->value() ), font, color ) );
  }
  else
  {
    return new BError( "A parameter was invalid" );
  }
}

BObjectImp* UnicodeExecutorModule::mf_RequestInputUC()
{
  Mobile::Character* chr;
  Items::Item* item;
  const String* prompt;
  const String* lang;
  if ( getCharacterParam( 0, chr ) && getItemParam( 1, item ) &&
       getUnicodeStringParam( 2, prompt ) && getStringParam( 3, lang ) )
  {
    if ( !chr->has_active_client() )
    {
      return new BError( "No client attached" );
    }

    if ( chr->has_active_prompt() )
    {
      return new BError( "Another script has an active prompt" );
    }

    if ( prompt->length() > SPEECH_MAX_LEN )
      return new BError( "Prompt exceeds maximum size." );
    if ( lang->length() != 3 )
      return new BError( "langcode must be a 3-character code." );

    if ( !uoexec.suspend() )
    {
      DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
               << "\tCall to function Unicode::RequestInputUC():\n"
               << "\tThe execution of this script can't be blocked!\n";
      return new Bscript::BError( "Script can't be blocked" );
    }

    Core::send_sysmessage_unicode( chr->client, prompt->value(),
                                   Clib::strupperASCII( lang->value() ) );

    chr->client->gd->prompt_uniemod = this;
    prompt_chr = chr;

    Core::send_unicode_prompt( chr->client, ctBEu32( item->serial ) );

    return new BLong( 0 );
  }
  else
  {
    return new BError( "A parameter was invalid" );
  }
}

BObjectImp* UnicodeExecutorModule::mf_SendSysMessageUC()
{
  Mobile::Character* chr;
  const String* text;
  const String* lang;
  unsigned short font;
  unsigned short color;

  if ( getCharacterParam( 0, chr ) && getUnicodeStringParam( 1, text ) &&
       getStringParam( 2, lang ) && getParam( 3, font ) && getParam( 4, color ) )
  {
    if ( chr->has_active_client() )
    {
      if ( text->length() > SPEECH_MAX_LEN )
        return new BError( "Text exceeds maximum size." );
      if ( lang->length() != 3 )
        return new BError( "langcode must be a 3-character code." );

      Core::send_sysmessage_unicode( chr->client, text->value(),
                                     Clib::strupperASCII( lang->value() ), font, color );
      return new BLong( 1 );
    }
    else
    {
      return new BError( "Mobile has no active client" );
    }
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* UnicodeExecutorModule::mf_SendTextEntryGumpUC()
{
  // SendTextEntryGumpUC(who, uc_text1, cancel := TE_CANCEL_ENABLE,
  //            style := TE_STYLE_NORMAL, maximum := 40, uc_text2 := {} );

  return new BError( "Function not implimented" );
}

size_t UnicodeExecutorModule::sizeEstimate() const
{
  return sizeof( *this );
}
}  // namespace Module
}  // namespace Pol
