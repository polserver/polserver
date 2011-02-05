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

#include <iosfwd>

class BasicIoExecutorModule;

typedef BObjectImp* (BasicIoExecutorModule::*BasicIoExecutorModuleFn)();

#ifdef _MSC_VER
#	pragma pack( push, 1 )
#else
/* Ok, my build of GCC supports this, yay! */
#	pragma pack(1)
#endif
struct BasicIoFunctionDef
{
	char funcname[ 33 ];
	BasicIoExecutorModuleFn fptr;
};
#ifdef _MSC_VER
#	pragma pack( pop )
#else
#	pragma pack()
#endif
 

class BasicIoExecutorModule : public ExecutorModule
{
  public:
    BObjectImp* print();

    BasicIoExecutorModule(Executor& exec,
                          std::ostream& cout) : ExecutorModule("BasicIo", exec),
                                                cout_(cout) { }

	// class machinery
  protected:
	virtual BObjectImp* execFunc( unsigned idx );
	virtual int functionIndex( const char *func );
	static BasicIoFunctionDef function_table[];
  private:
    std::ostream& cout_;
};

#endif
