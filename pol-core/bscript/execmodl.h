/*
History
=======
2006/10/28 Shinigami: GCC 4.1.1 fix - invalid use of constructor as a template

Notes
=======

*/

#ifndef BSCRIPT_EXECMODL_H
#define BSCRIPT_EXECMODL_H

#ifndef BSCRIPT_BOBJECT_H
#include "bobject.h"
#endif

#include "executor.h"
#include "../clib/boostutils.h"

#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

namespace Pol {
  namespace Bscript {

	class Executor;
	class Token;
	class String;
	class ExecutorModule;

	typedef BObject * ( ExecutorModule::*ExecutorModuleFn )( );

	class ExecutorModule
	{
	public:
	  virtual ~ExecutorModule() {};

	  BObjectImp* getParamImp( unsigned param );
	  BObjectImp* getParamImp( unsigned param, BObjectImp::BObjectType type );
	  bool getParamImp( unsigned param, BObjectImp*& imp );

	  const String* getStringParam( unsigned param );
	  void* getApplicPtrParam( unsigned param, const BApplicObjType* pointer_type );
	  BApplicObjBase* getApplicObjParam( unsigned param, const BApplicObjType* object_type );
	  bool getStringParam( unsigned param, const String*& pstr );
	  bool getRealParam( unsigned param, double& value );
	  bool getObjArrayParam( unsigned param, ObjArray*& pobjarr );

	  bool getParam( unsigned param, int& value );
	  bool getParam( unsigned param, int& value, int maxval );
	  bool getParam( unsigned param, int& value, int minval, int maxval );

	  bool getParam( unsigned param, unsigned& value );

	  bool getParam( unsigned param, short& value );
	  bool getParam( unsigned param, short& value, short maxval );
	  bool getParam( unsigned param, short& value, short minval, short maxval );

	  bool getParam( unsigned param, unsigned short& value );
	  bool getParam( unsigned param, unsigned short& value, unsigned short maxval );
	  bool getParam( unsigned param, unsigned short& value, unsigned short minval, unsigned short maxval );

	  const std::string& scriptname() const;
	  Executor& exec;

	protected:
	  ExecutorModule( const char* moduleName, Executor& iExec );

	  boost_utils::function_name_flystring moduleName;

	  friend class Executor;

	  virtual int functionIndex( const char *funcname ) = 0; // returns -1 on not found
	  virtual BObjectImp* execFunc( unsigned idx ) = 0;
	  virtual std::string functionName( unsigned idx ) = 0;

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
	  void register_function( const char *funcname, BObject( T::*fptr )( ) );

	public:
	  struct FunctionDef
	  {
		const char *funcname;
		BObjectImp* ( T::*fptr )( );
	  };
	  static FunctionDef function_table[];
	  static int function_table_size;

	private:
	  virtual int functionIndex( const char *funcname );
	  virtual BObjectImp* execFunc( unsigned idx );
	  virtual std::string functionName( unsigned idx );
	};

	template<class T>
	TmplExecutorModule<T>::TmplExecutorModule( const char *modname, Executor& ex ) :
	  ExecutorModule( modname, ex )
	{}

	template<class T>
	inline int TmplExecutorModule<T>::functionIndex( const char *name )
	{
	  for ( int idx = 0; idx < function_table_size; idx++ )
	  {
		if ( stricmp( name, function_table[idx].funcname ) == 0 )
		  return idx;
	  }
	  return -1;
	}

	template<class T>
	inline BObjectImp* TmplExecutorModule<T>::execFunc( unsigned funcidx )
	{
	  T* derived = static_cast<T*>( this );

	  return callMemberFunction( *derived, function_table[funcidx].fptr )( );
	};

	template<class T>
	inline std::string TmplExecutorModule<T>::functionName( unsigned idx )
	{
	  return function_table[idx].funcname;
	}

  }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
