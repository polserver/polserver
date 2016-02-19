/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_BASICMOD_H
#define BSCRIPT_BASICMOD_H

#ifndef BSCRIPT_EXECMODL_H
#include "../../bscript/execmodl.h"
#endif

// FIXME: this is currently misnamed, should be StringExecutorModule
namespace Pol
{
namespace Module
{
class BasicExecutorModule;

typedef Bscript::BObjectImp* ( BasicExecutorModule::*BasicExecutorModuleFn )();

#ifdef _MSC_VER
#pragma pack( push, 1 )
#else
/* Ok, my build of GCC supports this, yay! */
#pragma pack( 1 )
#endif
struct BasicFunctionDef
{
  char funcname[33];
  BasicExecutorModuleFn fptr;
};
#ifdef _MSC_VER
#pragma pack( pop )
#else
#pragma pack()
#endif


class BasicExecutorModule : public Bscript::ExecutorModule
{
public:
  /* These probably belong in a string module */
  Bscript::BObjectImp* find();
  Bscript::BObjectImp* len();
  Bscript::BObjectImp* upper();
  Bscript::BObjectImp* lower();
  Bscript::BObjectImp* mf_substr();
  Bscript::BObjectImp* mf_Trim();
  Bscript::BObjectImp* mf_StrReplace();
  Bscript::BObjectImp* mf_SubStrReplace();
  Bscript::BObjectImp* mf_Compare();
  Bscript::BObjectImp* mf_CInt();
  Bscript::BObjectImp* mf_CStr();
  Bscript::BObjectImp* mf_CDbl();
  Bscript::BObjectImp* mf_CAsc();
  Bscript::BObjectImp* mf_CAscZ();
  Bscript::BObjectImp* mf_CChr();
  Bscript::BObjectImp* mf_CChrZ();

  Bscript::BObjectImp* mf_Bin();
  Bscript::BObjectImp* mf_Hex();
  Bscript::BObjectImp* mf_SplitWords();

  Bscript::BObjectImp* mf_Pack();
  Bscript::BObjectImp* mf_Unpack();

  Bscript::BObjectImp* mf_TypeOf();
  Bscript::BObjectImp* mf_SizeOf();
  Bscript::BObjectImp* mf_TypeOfInt();

  BasicExecutorModule( Bscript::Executor& exec ) : ExecutorModule( "Basic", exec ) {}
  // class machinery
protected:
  virtual Bscript::BObjectImp* execFunc( unsigned idx ) POL_OVERRIDE;
  virtual int functionIndex( const char* func ) POL_OVERRIDE;
  virtual std::string functionName( unsigned idx ) POL_OVERRIDE;
  static BasicFunctionDef function_table[];
};
}
}
#endif
