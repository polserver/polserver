/** @file
 *
 * @par History
 */


#include "basiciomod.h"

#include "../../clib/logfacility.h"

#include "../../clib/clib.h"
#include "../../clib/rawtypes.h"

#include "../../bscript/bobject.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"

#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#pragma warning(disable:4996) // stricmp deprecation
#endif

namespace Pol
{
namespace Module
{

Bscript::BObjectImp* BasicIoExecutorModule::print()
{
  INFO_PRINT << exec.getParamImp( 0 )->getStringRep() << "\n";
  return new Bscript::UninitObject;
}

BasicIoFunctionDef BasicIoExecutorModule::function_table[] =
{
  { "print", &BasicIoExecutorModule::print }
};

int BasicIoExecutorModule::functionIndex( const char* name )
{
  for ( unsigned idx = 0; idx < arsize( function_table ); idx++ )
  {
    if ( stricmp( name, function_table[idx].funcname ) == 0 )
      return idx;
  }
  return -1;
}

Bscript::BObjectImp* BasicIoExecutorModule::execFunc( unsigned funcidx )
{
  return callMemberFunction( *this, function_table[funcidx].fptr )( );
};

std::string BasicIoExecutorModule::functionName( unsigned idx )
{
  return function_table[idx].funcname;
}


#if 0
Object* FileExecutorModule::execFunc(const Token& token)
{
  int res = 0;
  Object* result = NULL;
  switch(token.id)
  {
  case FILE_OPEN:
    break;
  case FILE_CLOSE:
  case FILE_READ:
  case FILE_WRITE:
  case FILE_SEEK:
  case FILE_TELL:
  case FILE_CHSIZE:
    Object* obj;

    obj = getParam(0);
    if (!obj->isA("File"))
    {
      res = -1;
      break;
    }
    OFile* of;
    of = (OFile*) obj;
    switch(token.id)
    {
    case FILE_READ:
    case FILE_WRITE:
    case FILE_SEEK:
    case FILE_TELL:
    case FILE_CHSIZE:
      int newLen = (int)doubleParam(1);
      res = of->chsize(newLen);
      break;
    }
    break;
  default:
    passert(0);
    break;
  }
  if (result == NULL) result = new Double((double)res);

  return result;
}
#endif
}
}