/** @file
 *
 * @par History
 * - 2006/10/28 Shinigami: GCC 4.1.1 fix - invalid use of constructor as a template
 */


#ifndef BSCRIPT_EXECMODL_H
#define BSCRIPT_EXECMODL_H

#ifndef BSCRIPT_BOBJECT_H
#include "bobject.h"
#endif

#include <map>
#include <string>
#include <vector>

#include "../clib/boostutils.h"
#include "../clib/maputil.h"
#include "executor.h"

namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Bscript
{
class ExecutorModule;
class String;

typedef BObject* ( ExecutorModule::*ExecutorModuleFn )();

class ExecutorModule
{
public:
  virtual ~ExecutorModule() = default;

  BObjectImp* getParamImp( unsigned param );
  BObjectImp* getParamImp( unsigned param, BObjectImp::BObjectType type );
  bool getParamImp( unsigned param, BObjectImp*& imp );

  const String* getStringParam( unsigned param );
  void* getApplicPtrParam( unsigned param, const BApplicObjType* pointer_type );
  BApplicObjBase* getApplicObjParam( unsigned param, const BApplicObjType* object_type );
  bool getStringParam( unsigned param, const String*& pstr );
  bool getUnicodeStringParam( unsigned param, const String*& pstr );  // accepts also BLong array
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
  bool getParam( unsigned param, unsigned short& value, unsigned short minval,
                 unsigned short maxval );

  const std::string& scriptname() const;
  Executor& exec;

  virtual size_t sizeEstimate() const;

protected:
  ExecutorModule( const char* moduleName, Executor& iExec );

  boost_utils::function_name_flystring moduleName;

  friend class Executor;

  virtual int functionIndex( const std::string& funcname ) = 0;  // returns -1 on not found
  virtual BObjectImp* execFunc( unsigned idx ) = 0;
  virtual std::string functionName( unsigned idx ) = 0;

private:  // not implemented
  ExecutorModule( const ExecutorModule& exec ) = delete;
  ExecutorModule& operator=( const ExecutorModule& exec ) = delete;
};

// FIXME: this function doesn't seem to work.
template <class T>
BApplicObj<T>* getApplicObjParam( ExecutorModule& ex, unsigned param,
                                  const BApplicObjType* object_type )
{
  return static_cast<BApplicObj<T>*>( ex.getApplicObjParam( param, object_type ) );
}

template <class T>
class TmplExecutorModule : public ExecutorModule
{
public:
  static const char* const modname;

protected:
  TmplExecutorModule( Executor& exec );

private:
  struct FunctionDef
  {
    std::string funcname;
    BObjectImp* ( T::*fptr )();
    unsigned argc;
  };
  using FunctionTable = std::vector<FunctionDef>;

  static FunctionTable function_table;
  static std::map<std::string, int, Clib::ci_cmp_pred> _func_idx_map;
  static bool _func_map_init;

protected:
  virtual int functionIndex( const std::string& funcname ) override;
  virtual BObjectImp* execFunc( unsigned idx ) override;
  virtual std::string functionName( unsigned idx ) override;
};

template <class T>
std::map<std::string, int, Clib::ci_cmp_pred> TmplExecutorModule<T>::_func_idx_map;

template <class T>
bool TmplExecutorModule<T>::_func_map_init = false;

template <class T>
TmplExecutorModule<T>::TmplExecutorModule( Executor& ex )
    : ExecutorModule( TmplExecutorModule::modname, ex )
{
  if ( !_func_map_init )
  {
    for ( unsigned idx = 0; idx < function_table.size(); idx++ )
    {
      _func_idx_map[function_table[idx].funcname] = idx;
    }
    _func_map_init = true;
  }
}

template <class T>
inline int TmplExecutorModule<T>::functionIndex( const std::string& name )
{
  auto itr = _func_idx_map.find( name );
  if ( itr != _func_idx_map.end() )
    return itr->second;
  return -1;
}

template <class T>
inline BObjectImp* TmplExecutorModule<T>::execFunc( unsigned funcidx )
{
  T* derived = static_cast<T*>( this );
  return ( ( *derived ).*( function_table[funcidx].fptr ) )();
};

template <class T>
inline std::string TmplExecutorModule<T>::functionName( unsigned idx )
{
  return function_table[idx].funcname;
}
}  // namespace Bscript
}  // namespace Pol

#endif
