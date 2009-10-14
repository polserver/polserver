/*
History
=======
2006/10/28 Shinigami: GCC 4.1.1 fix - invalid use of constructor as a template

Notes
=======

*/

#ifndef __EXECMODL_H
#define __EXECMODL_H

class Executor;
class Token;

#include "bobject.h"
#include "executor.h"

class String;
class ExecutorModule;

typedef BObject * (ExecutorModule::*ExecutorModuleFn)();

class ExecutorModule
{
public:
    virtual ~ExecutorModule() {};

    BObjectImp* getParamImp(unsigned param) { return exec.getParamImp( param ); }
    BObjectImp* getParamImp(unsigned param, BObjectImp::BObjectType type) { return exec.getParamImp( param, type ); }
    const String* getStringParam(unsigned param) { return exec.getStringParam( param ); }
    void* getApplicPtrParam( unsigned param, const BApplicObjType* pointer_type ) { return exec.getApplicPtrParam( param, pointer_type ); }
    BApplicObjBase* getApplicObjParam( unsigned param, const BApplicObjType* object_type ) { return exec.getApplicObjParam( param, object_type ); }
    bool getStringParam( unsigned param, const String*& pstr ) { return exec.getStringParam( param, pstr ); }
    bool getRealParam( unsigned param, double& value ) { return exec.getRealParam( param, value ); }
    bool getObjArrayParam( unsigned param, ObjArray*& pobjarr ) { return exec.getObjArrayParam( param, pobjarr ); }

    bool getParam( unsigned param, long& value ) 
    { 
        return exec.getParam( param, value ); 
    }
    bool getParam( unsigned param, long& value, long maxval ) 
    { 
        return exec.getParam( param, value, maxval ); 
    }
    bool getParam( unsigned param, long& value, long minval, long maxval ) 
    { 
        return exec.getParam( param, value, minval, maxval ); 
    }

    bool getParam( unsigned param, unsigned& value ) 
    { 
        return exec.getParam( param, value ); 
    }

	bool getParam( unsigned param, short& value ) 
    { 
        return exec.getParam( param, value ); 
    }
	bool getParam( unsigned param, short& value, short maxval ) 
    { 
        return exec.getParam( param, value, maxval ); 
    }
	bool getParam( unsigned param, short& value, short minval, short maxval ) 
    { 
        return exec.getParam( param, value, minval, maxval ); 
    }

    bool getParam( unsigned param, unsigned short& value ) 
    { 
        return exec.getParam( param, value ); 
    }
    bool getParam( unsigned param, unsigned short& value, unsigned short maxval ) 
    { 
        return exec.getParam( param, value, maxval ); 
    }
    bool getParam( unsigned param, unsigned short& value, unsigned short minval, unsigned short maxval ) 
    { 
        return exec.getParam( param, value, minval, maxval ); 
    }

    const string& scriptname() const;
	Executor& exec;

protected:
    ExecutorModule(const char* moduleName, Executor& iExec) :
       exec(iExec), 
	   moduleName(moduleName) 
	   { }

    const char* moduleName;

	friend class Executor;

	virtual int functionIndex( const char *funcname ) = 0; // returns -1 on not found
	virtual BObjectImp* execFunc( unsigned idx ) = 0;

private: // not implemented
    ExecutorModule( const ExecutorModule& exec );
    ExecutorModule& operator=( const ExecutorModule& exec );
};

// FIXME: this function doesn't seem to work.
template<class T>
BApplicObj<T>* getApplicObjParam( ExecutorModule& ex, unsigned param, const BApplicObjType* object_type )
{
	return static_cast<BApplicObj<T>*>( ex.getApplicObjParam( param, object_type ) );
}

#define callMemberFunction(object,ptrToMember) ((object).*(ptrToMember))

template<class T>
class TmplExecutorModule : public ExecutorModule
{
protected:
    TmplExecutorModule( const char* modname, Executor& exec );
    void register_function( const char *funcname, BObject (T::*fptr)() );

public:
    struct FunctionDef 
    {
        const char *funcname;
        BObjectImp* (T::*fptr)();
    };
    static FunctionDef function_table[];
    static int function_table_size;

private:
    virtual int functionIndex( const char *funcname );
    virtual BObjectImp* execFunc( unsigned idx );
};

template<class T>
TmplExecutorModule<T>::TmplExecutorModule( const char *modname, Executor& ex ) :
    ExecutorModule( modname, ex )
{
}

template<class T>
int TmplExecutorModule<T>::functionIndex( const char *name )
{
	for( int idx = 0; idx < function_table_size; idx++ )
	{
		if (stricmp( name, function_table[idx].funcname ) == 0)
			return idx;
	}
	return -1;
}

template<class T>
BObjectImp* TmplExecutorModule<T>::execFunc( unsigned funcidx )
{
    T* derived = static_cast<T*>(this);

	return callMemberFunction(*derived, function_table[funcidx].fptr)();
};



#endif
