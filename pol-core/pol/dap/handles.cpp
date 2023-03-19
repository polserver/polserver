#include "handles.h"

#include "../../bscript/bstruct.h"
#include "../../bscript/dict.h"

#include <dap/protocol.h>

namespace Pol
{
namespace Network
{
namespace DAP
{
using namespace Bscript;

Handles::Handles() : _nextHandle( START_HANDLE ) {}

void Handles::reset()
{
  _nextHandle = START_HANDLE;
  _handleMap.clear();
}

int Handles::create( const Reference& value )
{
  int handle = _nextHandle++;
  _handleMap.insert( { handle, value } );
  return handle;
}

Reference* Handles::get( int handle )
{
  auto iter = _handleMap.find( handle );
  if ( iter != _handleMap.end() )
  {
    return &iter->second;
  }
  return nullptr;
}

void Handles::add_variable_details( const Bscript::BObjectRef& objref, dap::Variable& variable )
{
  auto impptr = objref->impptr();

  switch ( impptr->type() )
  {
  case BObjectImp::BObjectType::OTString:
    variable.type = "string";
    variable.value = impptr->getStringRep();
    break;
  case BObjectImp::BObjectType::OTDouble:
  case BObjectImp::BObjectType::OTLong:
    variable.type = "number";
    variable.value = impptr->getStringRep();
    break;
  case BObjectImp::BObjectType::OTStruct:
    variable.type = "struct";
    variable.value = "struct{ ... }";
    variable.namedVariables = static_cast<BStruct*>( impptr )->contents().size();
    variable.variablesReference = create( objref );
    break;
  case BObjectImp::BObjectType::OTDictionary:
    variable.type = "dictionary";
    variable.value = "dictionary{ ... }";
    variable.namedVariables = static_cast<BDictionary*>( impptr )->contents().size();
    variable.variablesReference = create( objref );
    break;
  case BObjectImp::BObjectType::OTArray:
    variable.type = "array";
    variable.value = "{ ... }";
    variable.indexedVariables = static_cast<ObjArray*>( impptr )->ref_arr.size();
    variable.variablesReference = create( objref );
    break;
  default:
    variable.value = impptr->getStringRep();
    break;
  }
}
}  // namespace DAP
}  // namespace Network
}  // namespace Pol
