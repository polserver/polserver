/** @file
 *
 * @par History
 */


#include "execmodl.h"

#include <string>

#include "executor.h"


namespace Pol::Bscript
{
ExecutorModule::ExecutorModule( const char* moduleName, Executor& iExec )
    : exec( iExec ), moduleName( moduleName )
{
}

BObjectImp* ExecutorModule::getParamImp( unsigned param )
{
  return exec.getParamImp( param );
}
BObjectImp* ExecutorModule::getParamImp( unsigned param, BObjectImp::BObjectType type )
{
  return exec.getParamImp( param, type );
}
bool ExecutorModule::getParamImp( unsigned param, BObjectImp*& imp )
{
  imp = exec.getParamImp( param );
  return imp != nullptr;
}

const String* ExecutorModule::getStringParam( unsigned param )
{
  return exec.getStringParam( param );
}
void* ExecutorModule::getApplicPtrParam( unsigned param, const BApplicObjType* pointer_type )
{
  return exec.getApplicPtrParam( param, pointer_type );
}
BApplicObjBase* ExecutorModule::getApplicObjParam( unsigned param,
                                                   const BApplicObjType* object_type )
{
  return exec.getApplicObjParam( param, object_type );
}
bool ExecutorModule::getStringParam( unsigned param, const String*& pstr )
{
  return exec.getStringParam( param, pstr );
}
bool ExecutorModule::getUnicodeStringParam( unsigned param, const String*& pstr )
{
  return exec.getUnicodeStringParam( param, pstr );
}
bool ExecutorModule::getRealParam( unsigned param, double& value )
{
  return exec.getRealParam( param, value );
}
bool ExecutorModule::getObjArrayParam( unsigned param, ObjArray*& pobjarr )
{
  return exec.getObjArrayParam( param, pobjarr );
}

bool ExecutorModule::getParam( unsigned param, int& value )
{
  return exec.getParam( param, value );
}
bool ExecutorModule::getParam( unsigned param, int& value, int maxval )
{
  return exec.getParam( param, value, maxval );
}
bool ExecutorModule::getParam( unsigned param, int& value, int minval, int maxval )
{
  return exec.getParam( param, value, minval, maxval );
}

bool ExecutorModule::getParam( unsigned param, unsigned& value )
{
  return exec.getParam( param, value );
}

bool ExecutorModule::getParam( unsigned param, short& value )
{
  return exec.getParam( param, value );
}
bool ExecutorModule::getParam( unsigned param, short& value, short maxval )
{
  return exec.getParam( param, value, maxval );
}
bool ExecutorModule::getParam( unsigned param, short& value, short minval, short maxval )
{
  return exec.getParam( param, value, minval, maxval );
}

bool ExecutorModule::getParam( unsigned param, unsigned short& value )
{
  return exec.getParam( param, value );
}
bool ExecutorModule::getParam( unsigned param, unsigned short& value, unsigned short maxval )
{
  return exec.getParam( param, value, maxval );
}
bool ExecutorModule::getParam( unsigned param, unsigned short& value, unsigned short minval,
                               unsigned short maxval )
{
  return exec.getParam( param, value, minval, maxval );
}

bool ExecutorModule::getParam( unsigned param, bool& value )
{
  return exec.getParam( param, value );
}

const std::string& ExecutorModule::scriptname() const
{
  return exec.prog_->name;
}

size_t ExecutorModule::sizeEstimate() const
{
  return sizeof( *this );
}
}  // namespace Pol::Bscript
