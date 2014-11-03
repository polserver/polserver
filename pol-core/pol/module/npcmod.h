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
#include "../reftypes.h"
#include "../uconst.h"

#include <string>

namespace Pol {
  namespace Bscript {
	class BObjectImp;
  }
  namespace Core {
    class NPC;
  }

  namespace Module {
	class OSExecutorModule;
	class NPCExecutorModule;

	typedef Bscript::BObjectImp* ( NPCExecutorModule::*NPCExecutorModuleFn )( );

#ifdef _MSC_VER
#	pragma pack( push, 1 )
#else
	/* Ok, my build of GCC supports this, yay! */
#	pragma pack(1)
#endif
	struct NPCFunctionDef
	{
	  char funcname[33];
	  NPCExecutorModuleFn fptr;
	};
#ifdef _MSC_VER
#	pragma pack( pop )
#else
#	pragma pack()
#endif

	class NPCExecutorModule : public Bscript::ExecutorModule
	{
	public:
      NPCExecutorModule( Bscript::Executor& ex, Core::NPC& npc );
	  virtual ~NPCExecutorModule();

	  Core::NPCRef npcref;
	  Core::NPC& npc;

	protected:
	  OSExecutorModule* os_module;

	  Bscript::BObjectImp* mf_Wander();
	  Bscript::BObjectImp* mf_Self();
	  Bscript::BObjectImp* face();
	  Bscript::BObjectImp* move();
	  Bscript::BObjectImp* mf_WalkToward();
	  Bscript::BObjectImp* mf_RunToward();
	  Bscript::BObjectImp* mf_WalkAwayFrom();
	  Bscript::BObjectImp* mf_RunAwayFrom();
	  Bscript::BObjectImp* mf_TurnToward();
	  Bscript::BObjectImp* mf_TurnAwayFrom();

	  Bscript::BObjectImp* mf_WalkTowardLocation();
	  Bscript::BObjectImp* mf_RunTowardLocation();
	  Bscript::BObjectImp* mf_WalkAwayFromLocation();
	  Bscript::BObjectImp* mf_RunAwayFromLocation();
	  Bscript::BObjectImp* mf_TurnTowardLocation();
	  Bscript::BObjectImp* mf_TurnAwayFromLocation();


	  Bscript::BObjectImp* say();
	  Bscript::BObjectImp* SayUC();
	  Bscript::BObjectImp* position();
	  Bscript::BObjectImp* facing();
	  Bscript::BObjectImp* getproperty( /* propertyname */ );
	  Bscript::BObjectImp* setproperty( /* propertyname propertyvalue */ );
	  Bscript::BObjectImp* makeboundingbox( /* areastring */ );
	  Bscript::BObjectImp* IsLegalMove();
	  Bscript::BObjectImp* CanMove();
	  Bscript::BObjectImp* CreateBackpack();
	  Bscript::BObjectImp* CreateItem();
	  Bscript::BObjectImp* mf_SetOpponent();
	  Bscript::BObjectImp* mf_SetWarMode();
	  Bscript::BObjectImp* mf_SetAnchor();

	  Bscript::BObjectImp* move_self( Core::UFACING facing, bool run, bool adjust_ok = false );
	  // class machinery 
	  virtual Bscript::BObjectImp* execFunc( unsigned idx );
	  virtual int functionIndex( const char* func );
	  virtual std::string functionName( unsigned idx );
	  static NPCFunctionDef function_table[];

	  inline bool _internal_move( Core::UFACING facing, int run ); //DAVE
	};
  }
}
#endif
