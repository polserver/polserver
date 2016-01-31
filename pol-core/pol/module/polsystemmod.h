/** @file
 *
 * @par History
 * - 2009/11/30 Turley:    added MD5Encrypt(string)
 */


#ifndef POLSYSTEMEMOD_H
#define POLSYSTEMEMOD_H

#include "../../bscript/execmodl.h"
namespace Pol {
  namespace Module {
	class PolSystemExecutorModule : public Bscript::TmplExecutorModule<PolSystemExecutorModule>
	{
	public:
      PolSystemExecutorModule( Bscript::Executor& exec ) :
        Bscript::TmplExecutorModule<PolSystemExecutorModule>( "polsys", exec ) {};

      Bscript::BObjectImp* mf_IncRevision( /* uobject */ );
      Bscript::BObjectImp* mf_Packages( );
      Bscript::BObjectImp* mf_GetCmdLevelName( );
      Bscript::BObjectImp* mf_GetCmdLevelNumber( );
      Bscript::BObjectImp* mf_GetPackageByName( );
      Bscript::BObjectImp* mf_ListTextCommands( );
      Bscript::BObjectImp* mf_Realms( );
      Bscript::BObjectImp* mf_ReloadConfiguration( );
      Bscript::BObjectImp* mf_ReadMillisecondClock( );
      Bscript::BObjectImp* mf_ListenPoints( );
      Bscript::BObjectImp* mf_SetSysTrayPopupText( );
      Bscript::BObjectImp* mf_GetItemDescriptor( );
      Bscript::BObjectImp* mf_CreatePacket( );
      Bscript::BObjectImp* mf_AddRealm(/*name,base*/ );
      Bscript::BObjectImp* mf_DeleteRealm(/*name*/ );
      Bscript::BObjectImp* mf_MD5Encrypt(/*string*/ );
      Bscript::BObjectImp* mf_FormatItemDescription(/*string,amount,suffix*/ );
      Bscript::BObjectImp* mf_LogCPropProfile();
	};
  }
}
#endif
