/*
History
=======
2009/12/21 Turley:    ._method() call fix


Notes
=======

*/

#ifndef EXSCROBJ_H
#define EXSCROBJ_H


#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

#include "../clib/weakptr.h"
namespace Pol {
  namespace Core {
	class UOExecutor;

	extern Bscript::BApplicObjType scriptexobjimp_type;
	typedef weak_ptr<UOExecutor> ScriptExPtr;
	class ScriptExObjImp : public Bscript::BApplicObj< ScriptExPtr >
	{
	  typedef Bscript::BApplicObj< ScriptExPtr > base;
	public:
	  explicit ScriptExObjImp( UOExecutor* uoexec );
	  virtual const char* typeOf() const POL_OVERRIDE;
	  virtual u8 typeOfInt() const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* copy( ) const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false ) POL_OVERRIDE;
	  virtual Bscript::BObjectRef get_member( const char* membername ) POL_OVERRIDE;
	  virtual Bscript::BObjectRef get_member_id( const int id ) POL_OVERRIDE;
	};
  }
}
#endif
