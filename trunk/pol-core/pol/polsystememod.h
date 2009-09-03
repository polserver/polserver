/*
History
=======


Notes
=======

*/

#ifndef POLSYSTEMEMOD_H
#define POLSYSTEMEMOD_H

#include "bscript/execmodl.h"

class PolSystemExecutorModule : public TmplExecutorModule<PolSystemExecutorModule>
{
public:
    PolSystemExecutorModule( Executor& exec ) :
      TmplExecutorModule<PolSystemExecutorModule>( "polsys", exec ) {};

	BObjectImp* mf_IncRevision( /* uobject */ );
    BObjectImp* mf_Packages();
	BObjectImp* mf_GetCmdLevelName();
	BObjectImp* mf_GetCmdLevelNumber();
    BObjectImp* mf_GetPackageByName();
	BObjectImp* mf_ListTextCommands();
    BObjectImp* mf_Realms();
    BObjectImp* mf_ReloadConfiguration();
    BObjectImp* mf_ReadMillisecondClock();
    BObjectImp* mf_ListenPoints();
    BObjectImp* mf_SetSysTrayPopupText();
    BObjectImp* mf_GetItemDescriptor();
    BObjectImp* mf_CreatePacket();
	BObjectImp* mf_AddRealm(/*name,base*/);
	BObjectImp* mf_DeleteRealm(/*name*/);
};

#endif
