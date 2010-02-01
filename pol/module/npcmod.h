/*
History
=======
2005/06/15 Shinigami: added CanMove - checks if an NPC can move in given direction
                      (IsLegalMove works in a different way and is used for bounding boxes only)


Notes
=======

*/

#ifndef NPCEMOD_H
#define NPCEMOD_H

#include "../../bscript/execmodl.h"

#include "../../clib/random.h"

class NPC;
class OSExecutorModule;
class BObjectImp;
class NPCExecutorModule;

typedef BObjectImp* (NPCExecutorModule::*NPCExecutorModuleFn)();

#ifdef _MSC_VER
#	pragma pack( push, 1 )
#else
/* Ok, my build of GCC supports this, yay! */
#	pragma pack(1)
#endif
struct NPCFunctionDef 
{
	char funcname[ 33 ];
	NPCExecutorModuleFn fptr;
};
#ifdef _MSC_VER
#	pragma pack( pop )
#else
#	pragma pack()
#endif

class NPCExecutorModule : public ExecutorModule
{
public:
	NPCExecutorModule( Executor& ex, NPC& npc );
    virtual ~NPCExecutorModule();

    NPCRef npcref;
    NPC& npc;

protected:
	OSExecutorModule* os_module;
    
    BObjectImp* mf_Wander();
    BObjectImp* mf_Self();
	BObjectImp* face();
	BObjectImp* move();
    BObjectImp* mf_WalkToward();
    BObjectImp* mf_RunToward();
    BObjectImp* mf_WalkAwayFrom();
    BObjectImp* mf_RunAwayFrom();
    BObjectImp* mf_TurnToward();
    BObjectImp* mf_TurnAwayFrom();

    BObjectImp* mf_WalkTowardLocation();
    BObjectImp* mf_RunTowardLocation();
    BObjectImp* mf_WalkAwayFromLocation();
    BObjectImp* mf_RunAwayFromLocation();
    BObjectImp* mf_TurnTowardLocation();
    BObjectImp* mf_TurnAwayFromLocation();


    BObjectImp* say();
    BObjectImp* SayUC();
    BObjectImp* position();
    BObjectImp* facing();
    BObjectImp* getproperty( /* propertyname */ );
    BObjectImp* setproperty( /* propertyname propertyvalue */ );
    BObjectImp* makeboundingbox( /* areastring */ );
    BObjectImp* IsLegalMove();
    BObjectImp* CanMove();
    BObjectImp* CreateBackpack();
    BObjectImp* CreateItem();
    BObjectImp* mf_SetOpponent();
    BObjectImp* mf_SetWarMode();
    BObjectImp* mf_SetAnchor();

    BObjectImp* move_self( UFACING facing, bool run, bool adjust_ok = false );
	// class machinery 
	virtual BObjectImp* execFunc( unsigned idx );
	virtual int functionIndex( const char* func );
	static NPCFunctionDef function_table[];

	inline bool _internal_move(UFACING facing, int run); //DAVE
};

#endif
