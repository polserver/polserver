#include "handles.h"

#include "../../bscript/bstruct.h"
#include "../../bscript/dict.h"
#include "../../clib/strutil.h"

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

dap::array<dap::Variable> Handles::to_variables( const Bscript::BObjectRef& objref )
{
  dap::array<dap::Variable> variables;
  auto impptr = objref->impptr();

  if ( impptr != nullptr )
  {
    if ( impptr->isa( BObjectImp::OTStruct ) )
    {
      BStruct* bstruct = static_cast<BStruct*>( impptr );

      for ( const auto& content : bstruct->contents() )
      {
        dap::Variable current_var;
        current_var.name = content.first;
        add_variable_details( content.second, current_var );
        variables.push_back( current_var );
      }
    }
    else if ( impptr->isa( BObjectImp::OTDictionary ) )
    {
      BDictionary* dict = static_cast<Bscript::BDictionary*>( impptr );
      for ( const auto& content : dict->contents() )
      {
        dap::Variable current_var;
        current_var.name = content.first->getStringRep();
        add_variable_details( content.second, current_var );
        variables.push_back( current_var );
      }
    }
    else if ( impptr->isa( BObjectImp::OTArray ) )
    {
      ObjArray* objarr = static_cast<Bscript::ObjArray*>( impptr );
      size_t index = 1;
      for ( const auto& content : objarr->ref_arr )
      {
        dap::Variable current_var;
        current_var.name = Clib::tostring( index++ );
        add_variable_details( content, current_var );
        variables.push_back( current_var );
      }
    }
  }
  return variables;
}
}  // namespace DAP
}  // namespace Network
}  // namespace Pol
