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
#include "../unicode.h"
#include "../uoexec.h"

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
  Module::UnicodeExecutorModule* uniemod = client->gd->prompt_uniemod;
  if ( uniemod == nullptr )
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

  bool ok = true;
  Bscript::BObject* valstack = nullptr;

#if ( 0 )
  // client version of the packet should always send this as a 1??
  if ( cfBEu16( msg->unk ) != 0x01 )
  {
    // ENHANCE: May want to log this, too?
    ok = false;
    valstack = new BObject( new BError( "Malformed return-packet from client" ) );
  }
#endif

  char lang[4];
  memcpy( lang, msg->lang, 4 );

  if ( ok )
  {
    Bscript::ObjArray uc_text;
    int i;
    for ( i = 0; i < textlen; i++ )
    {
      u16 wc = msg->wtext[i];                   // its not flipped!! ...i hate osi...
      if ( wc < (u16)0x20 || wc == (u16)0x7F )  // control character! >_<
      {
        ok = false;
        valstack = new Bscript::BObject(
            new Bscript::BError( "Invalid control characters in text entry" ) );

        POLLOG_ERROR << "Client #" << static_cast<unsigned long>( client->instance_ )
                     << " (account "
                     << ( ( client->acct != nullptr ) ? client->acct->name() : "unknown" )
                     << ") sent invalid unicode control characters (RequestInputUC)\n";
        break;  // for
      }
      uc_text.addElement( new Bscript::BLong( wc ) );
    }

    if ( ok )
    {
      if ( uc_text.ref_arr.empty() )
        valstack = new Bscript::BObject( new Bscript::BLong( 0 ) );
      else
      {
        std::unique_ptr<Bscript::BStruct> retval( new Bscript::BStruct() );
        retval->addMember( "lang", new Bscript::String( lang ) );
        retval->addMember( "uc_text", uc_text.copy() );
        valstack = new Bscript::BObject( retval.release() );
      }
    }
  }

  uniemod->exec.ValueStack.back().set( valstack );  // error or struct, regardless.
  uniemod->uoexec.revive();
  uniemod->prompt_chr = nullptr;
  client->gd->prompt_uniemod = nullptr;
}

//////////////////////////////////////////////////////////////////////////
}  // namespace Core
namespace Bscript
{
using namespace Module;
template <>
TmplExecutorModule<UnicodeExecutorModule>::FunctionTable
    TmplExecutorModule<UnicodeExecutorModule>::function_table = {
        {"BroadcastUC", &UnicodeExecutorModule::mf_BroadcastUC},
        {"PrintTextAboveUC", &UnicodeExecutorModule::mf_PrintTextAboveUC},
        {"PrintTextAbovePrivateUC", &UnicodeExecutorModule::mf_PrivateTextAboveUC},
        {"RequestInputUC", &UnicodeExecutorModule::mf_RequestInputUC},
        {"SendSysMessageUC", &UnicodeExecutorModule::mf_SendSysMessageUC},
        {"SendTextEntryGumpUC", &UnicodeExecutorModule::mf_SendTextEntryGumpUC}};
}  // namespace Bscript
namespace Module
{
using namespace Bscript;
u16 gwtext[( SPEECH_MAX_LEN + 1 )];

UnicodeExecutorModule::UnicodeExecutorModule( Core::UOExecutor& exec )
    : TmplExecutorModule<UnicodeExecutorModule>( "unicode", exec ),
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
  using std::wcout;  // wcout rox :)

  ObjArray* oText;
  const String* lang;
  unsigned short font;
  unsigned short color;
  unsigned short requiredCmdLevel;
  if ( getObjArrayParam( 0, oText ) && getStringParam( 1, lang ) &&
       getParam( 2, font ) &&             // todo: getFontParam
       getParam( 3, color ) &&            // todo: getColorParam
       getParam( 4, requiredCmdLevel ) )  // todo: getRequiredCmdLevelParam
  {
    size_t textlen = oText->ref_arr.size();
    if ( textlen > SPEECH_MAX_LEN )
      return new BError( "Unicode array exceeds maximum size." );
    if ( lang->length() != 3 )
      return new BError( "langcode must be a 3-character code." );
    // lang->toUpper(); // Language codes are in upper-case :)
    if ( !Core::convertArrayToUC( oText, gwtext, textlen ) )
      return new BError( "Invalid value in Unicode array." );
    Core::broadcast( gwtext, Clib::strupper( lang->value() ).c_str(), font, color,
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
  ObjArray* oText;
  const String* lang;
  unsigned short font;
  unsigned short color;
  int journal_print;

  if ( getUObjectParam( 0, obj ) && getObjArrayParam( 1, oText ) && getStringParam( 2, lang ) &&
       getParam( 3, font ) && getParam( 4, color ) && getParam( 5, journal_print ) )
  {
    size_t textlen = oText->ref_arr.size();
    if ( textlen > SPEECH_MAX_LEN )
      return new BError( "Unicode array exceeds maximum size." );
    if ( lang->length() != 3 )
      return new BError( "langcode must be a 3-character code." );
    if ( !Core::convertArrayToUC( oText, gwtext, textlen ) )
      return new BError( "Invalid value in Unicode array." );

    return new BLong( say_above( obj, gwtext, Clib::strupper( lang->value() ).c_str(), font, color,
                                 journal_print ) );
  }
  else
  {
    return new BError( "A parameter was invalid" );
  }
}

BObjectImp* UnicodeExecutorModule::mf_PrivateTextAboveUC()
{
  Mobile::Character* chr;
  Core::UObject* obj;
  ObjArray* oText;
  const String* lang;
  unsigned short font;
  unsigned short color;

  if ( getUObjectParam( 0, obj ) && getObjArrayParam( 1, oText ) && getStringParam( 2, lang ) &&
       getCharacterParam( 3, chr ) && getParam( 4, font ) && getParam( 5, color ) )
  {
    size_t textlen = oText->ref_arr.size();
    if ( textlen > SPEECH_MAX_LEN )
      return new BError( "Unicode array exceeds maximum size." );
    if ( lang->length() != 3 )
      return new BError( "langcode must be a 3-character code." );
    // lang->toUpper(); // Language codes are in upper-case :)
    if ( !Core::convertArrayToUC( oText, gwtext, textlen ) )
      return new BError( "Invalid value in Unicode array." );

    return new BLong( private_say_above( chr, obj, gwtext, Clib::strupper( lang->value() ).c_str(),
                                         font, color ) );
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
  ObjArray* oPrompt;
  const String* lang;
  if ( getCharacterParam( 0, chr ) && getItemParam( 1, item ) && getObjArrayParam( 2, oPrompt ) &&
       getStringParam( 3, lang ) )
  {
    if ( !chr->has_active_client() )
    {
      return new BError( "No client attached" );
    }

    if ( chr->has_active_prompt() )
    {
      return new BError( "Another script has an active prompt" );
    }

    size_t textlen = oPrompt->ref_arr.size();
    if ( textlen > SPEECH_MAX_LEN )
      return new BError( "Unicode array exceeds maximum size." );
    if ( lang->length() != 3 )
      return new BError( "langcode must be a 3-character code." );
    if ( !Core::convertArrayToUC( oPrompt, gwtext, textlen ) )
      return new BError( "Invalid value in Unicode array." );

    if ( !uoexec.suspend() )
    {
      DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << exec.PC << ": \n"
               << "\tCall to function Unicode::RequestInputUC():\n"
               << "\tThe execution of this script can't be blocked!\n";
      return new Bscript::BError( "Script can't be blocked" );
    }

    Core::send_sysmessage( chr->client, gwtext, Clib::strupper( lang->value() ).c_str() );

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
  ObjArray* oText;
  const String* lang;
  unsigned short font;
  unsigned short color;

  if ( getCharacterParam( 0, chr ) && getObjArrayParam( 1, oText ) && getStringParam( 2, lang ) &&
       getParam( 3, font ) && getParam( 4, color ) )
  {
    if ( chr->has_active_client() )
    {
      size_t textlen = oText->ref_arr.size();
      if ( textlen > SPEECH_MAX_LEN )
        return new BError( "Unicode array exceeds maximum size." );
      if ( lang->length() != 3 )
        return new BError( "langcode must be a 3-character code." );
      // lang->toUpper(); // Language codes are in upper-case :)
      if ( !Core::convertArrayToUC( oText, gwtext, textlen ) )
        return new BError( "Invalid value in Unicode array." );

      Core::send_sysmessage( chr->client, gwtext, Clib::strupper( lang->value() ).c_str(), font,
                             color );
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
