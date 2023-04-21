#include "handles.h"

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
        set_response_details( content.second, current_var );
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
        set_response_details( content.second, current_var );
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
        set_response_details( content, current_var );
        variables.push_back( current_var );
      }
    }
    else if ( impptr->isa( BObjectImp::OTApplicObj ) )
    {
      for ( int i = 0; i < Bscript::n_objmembers; ++i )
      {
        const auto& object_member = Bscript::object_members[i];
        auto member_value = impptr->get_member_id( object_member.id );
        if ( !member_value->isa( BObjectImp::BObjectType::OTUninit ) )
        {
          dap::Variable variable;
          variable.name = object_member.code;
          set_response_details( member_value, variable );
          variables.push_back( variable );
        }
      }

      std::sort( variables.begin(), variables.end(),
                 []( const auto& a, const auto& b ) { return a.name < b.name; } );
    }
  }
  return variables;
}
}  // namespace DAP
}  // namespace Network
}  // namespace Pol
