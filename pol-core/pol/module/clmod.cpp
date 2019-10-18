/** @file
 *
 * @par History
 * - 2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 */


#include "clmod.h"
#include "../../bscript/berror.h"
#include "../../bscript/impstr.h"
#include "../../clib/passert.h"
#include "../../clib/rawtypes.h"
#include "../clfunc.h"
#include "../mobile/charactr.h"
#include "../network/pktdef.h"
#include "../uoexhelp.h"

namespace Pol
{
namespace Module
{
using namespace Bscript;

ClilocExecutorModule::ClilocExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<ClilocExecutorModule>( exec )
{
}

BObjectImp* ClilocExecutorModule::mf_SendSysMessageCL()
{
  Mobile::Character* chr;
  const String* text;
  unsigned int cliloc_num;
  unsigned short color;
  unsigned short font;

  if ( getCharacterParam( exec, 0, chr ) && getParam( 1, cliloc_num ) &&
       getUnicodeStringParam( 2, text ) && getParam( 3, font ) && getParam( 4, color ) )
  {
    passert_paranoid( chr != nullptr && text != nullptr );

    if ( !chr->has_active_client() )
      return new BError( "Mobile has no active client" );

    if ( text->length() > SPEECH_MAX_LEN )
      return new BError( "Text exceeds maximum size." );

    Core::send_sysmessage_cl( chr->client, cliloc_num, text->value(), font, color );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* ClilocExecutorModule::mf_PrintTextAboveCL()
{
  Core::UObject* obj;
  const String* text;
  unsigned int cliloc_num;
  unsigned short color;
  unsigned short font;

  if ( getUObjectParam( exec, 0, obj ) && getParam( 1, cliloc_num ) &&
       getUnicodeStringParam( 2, text ) && getParam( 3, font ) && getParam( 4, color ) )
  {
    passert_paranoid( text != nullptr && obj != nullptr );

    if ( text->length() > SPEECH_MAX_LEN )
      return new BError( "Text exceeds maximum size." );

    say_above_cl( obj, cliloc_num, text->value(), font, color );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* ClilocExecutorModule::mf_PrintTextAbovePrivateCL()
{
  Mobile::Character* chr;
  Core::UObject* obj;
  const String* text;
  unsigned int cliloc_num;
  unsigned short color;
  unsigned short font;

  if ( getCharacterParam( exec, 0, chr ) && getUObjectParam( exec, 1, obj ) &&
       getParam( 2, cliloc_num ) && getUnicodeStringParam( 3, text ) && getParam( 4, font ) &&
       getParam( 5, color ) )
  {
    passert_paranoid( chr != nullptr && text != nullptr && obj != nullptr );

    if ( !chr->has_active_client() )
      return new BError( "Mobile has no active client" );

    if ( chr->realm != obj->realm )
      return new BError( "Cannot print messages across realms!" );

    if ( text->length() > SPEECH_MAX_LEN )
      return new BError( "Text exceeds maximum size." );

    private_say_above_cl( chr, obj, cliloc_num, text->value(), font, color );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}
}  // namespace Module
}  // namespace Pol
