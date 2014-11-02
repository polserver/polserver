/*
History
=======

Notes
=======

*/

#ifndef __BSCRIPT_BASICIO_H
#define __BSCRIPT_BASICIO_H
#define __BASICIO_H

#ifndef __EXECMODL_H
#	include "../../bscript/execmodl.h"
#endif

#include <string>

namespace Pol {
  namespace Module {
	class BasicIoExecutorModule;

	typedef Bscript::BObjectImp* ( BasicIoExecutorModule::*BasicIoExecutorModuleFn )( );

#ifdef _MSC_VER
#	pragma pack( push, 1 )
#else
	/* Ok, my build of GCC supports this, yay! */
#	pragma pack(1)
#endif
	struct BasicIoFunctionDef
	{
	  char funcname[33];
	  BasicIoExecutorModuleFn fptr;
	};
#ifdef _MSC_VER
#	pragma pack( pop )
#else
#	pragma pack()
#endif


	class BasicIoExecutorModule : public Bscript::ExecutorModule
	{
	public:
	  Bscript::BObjectImp* print();

	  BasicIoExecutorModule( Bscript::Executor& exec) : Bscript::ExecutorModule( "BasicIo", exec )
	  {}

	  // class machinery
	protected:
	  virtual Bscript::BObjectImp* execFunc( unsigned idx );
	  virtual int functionIndex( const char *func );
	  virtual std::string functionName( unsigned idx );
	  static BasicIoFunctionDef function_table[];
	};
  }
}
#endif
