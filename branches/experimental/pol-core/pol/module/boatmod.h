/*
History
=======
2009/09/03 MuadDib:	  Changes for account related source file relocation
                      Changes for multi related source file relocation


Notes
=======

*/

#ifndef BOATEMOD_H
#define BOATEMOD_H

#include "../../bscript/execmodl.h"

class BObject;

class UBoatExecutorModule : public TmplExecutorModule<UBoatExecutorModule>
{
public:
    UBoatExecutorModule( Executor& exec ) :
        TmplExecutorModule<UBoatExecutorModule>( "boat", exec ) {};

    BObjectImp* mf_MoveBoat();
    BObjectImp* mf_MoveBoatRelative();

    BObjectImp* mf_TurnBoat();

    BObjectImp* mf_RegisterItemWithBoat();
    BObjectImp* mf_BoatFromItem();

    BObjectImp* mf_SystemFindBoatBySerial();

	BObjectImp* mf_MoveBoatXY();
};

#endif
