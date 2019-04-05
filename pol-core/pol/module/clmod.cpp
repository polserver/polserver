/** @file
 *
 * @par History
 * - 2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 */


#include "clmod.h"

#include "../../bscript/berror.h"
#include "../../clib/passert.h"
#include "../../clib/rawtypes.h"
#include "../clfunc.h"
#include "../mobile/charactr.h"
#include "../network/pktdef.h"
#include "../unicode.h"
#include "../uoexhelp.h"

namespace Pol
{
namespace Bscript
{
using namespace Module;
template <>
TmplExecutorModule<ClilocExecutorModule>::FunctionTable
    TmplExecutorModule<ClilocExecutorModule>::function_table = {

        {"SendSysMessageCL", &ClilocExecutorModule::mf_SendSysMessageCL, UINT_MAX},
        {"PrintTextAboveCL", &ClilocExecutorModule::mf_PrintTextAboveCL, UINT_MAX},
        {"PrintTextAbovePrivateCL", &ClilocExecutorModule::mf_PrintTextAbovePrivateCL, UINT_MAX}};
}  // namespace Bscript
namespace Module
{
using namespace Bscript;

ClilocExecutorModule::ClilocExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<ClilocExecutorModule>( "cliloc", exec )
{
}

BObjectImp* ClilocExecutorModule::mf_SendSysMessageCL()
{
  Mobile::Character* chr;
  ObjArray* oText;
  unsigned int cliloc_num;
  unsigned short color;
  unsigned short font;

  if ( getCharacterParam( exec, 0, chr ) && getParam( 1, cliloc_num ) &&
       getObjArrayParam( 2, oText ) && getParam( 3, font ) && getParam( 4, color ) )
  {
    passert_paranoid( chr != nullptr && oText != nullptr );

    if ( !chr->has_active_client() )
      return new BError( "Mobile has no active client" );

    if ( oText->ref_arr.size() > SPEECH_MAX_LEN )
      return new BError( "Unicode array exceeds maximum size." );

    u16 cltext[( SPEECH_MAX_LEN + 1 )];
    size_t textlen = oText->ref_arr.size();
    if ( !Core::convertArrayToUC( oText, cltext, textlen, false ) )
      return new BError( "Invalid value in Unicode array." );

    Core::send_sysmessage_cl( chr->client, cliloc_num, cltext, font, color );
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
  ObjArray* oText;
  unsigned int cliloc_num;
  unsigned short color;
  unsigned short font;

  if ( getUObjectParam( exec, 0, obj ) && getParam( 1, cliloc_num ) &&
       getObjArrayParam( 2, oText ) && getParam( 3, font ) && getParam( 4, color ) )
  {
    passert_paranoid( oText != nullptr && obj != nullptr );

    if ( oText->ref_arr.size() > SPEECH_MAX_LEN )
      return new BError( "Unicode array exceeds maximum size." );


    u16 cltext[( SPEECH_MAX_LEN + 1 )];
    size_t textlen = oText->ref_arr.size();
    if ( !Core::convertArrayToUC( oText, cltext, textlen, false ) )
      return new BError( "Invalid value in Unicode array." );

    say_above_cl( obj, cliloc_num, cltext, font, color );
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
  ObjArray* oText;
  unsigned int cliloc_num;
  unsigned short color;
  unsigned short font;

  if ( getCharacterParam( exec, 0, chr ) && getUObjectParam( exec, 1, obj ) &&
       getParam( 2, cliloc_num ) && getObjArrayParam( 3, oText ) && getParam( 4, font ) &&
       getParam( 5, color ) )
  {
    passert_paranoid( chr != nullptr && oText != nullptr && obj != nullptr );

    if ( !chr->has_active_client() )
      return new BError( "Mobile has no active client" );

    if ( chr->realm != obj->realm )
      return new BError( "Cannot print messages across realms!" );

    if ( oText->ref_arr.size() > SPEECH_MAX_LEN )
      return new BError( "Unicode array exceeds maximum size." );


    u16 cltext[( SPEECH_MAX_LEN + 1 )];
    size_t textlen = oText->ref_arr.size();
    if ( !Core::convertArrayToUC( oText, cltext, textlen, false ) )
      return new BError( "Invalid value in Unicode array." );

    private_say_above_cl( chr, obj, cliloc_num, cltext, font, color );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}
}  // namespace Module
}  // namespace Pol
