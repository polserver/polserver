#include "handles.h"

#include "../../bscript/eprog.h"
#include "../uoexec.h"


namespace Pol::Network::DAP
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

BObjectRef Handles::set_index_or_member( const BObjectRef& objref, const std::string& key,
                                         BObjectRef& value )
{
  auto impptr = objref->impptr();

  if ( impptr != nullptr )
  {
    if ( impptr->isa( BObjectImp::OTStruct ) )
    {
      BStruct* bstruct = static_cast<BStruct*>( impptr );
      bstruct->addMember( key.c_str(), value );
      return value;
    }
    if ( impptr->isa( BObjectImp::OTDictionary ) )
    {
      BDictionary* dict = static_cast<BDictionary*>( impptr );
      dict->addMember( key.c_str(), value );
      return value;
    }
    else if ( impptr->isa( BObjectImp::OTArray ) )
    {
      ObjArray* objarr = static_cast<ObjArray*>( impptr );
      auto index = strtoul( key.c_str(), nullptr, 0 );
      objarr->ref_arr.at( index ) = value;
      return value;
    }
    else if ( impptr->isa( BObjectImp::OTApplicObj ) )
    {
      impptr->set_member( key.c_str(), value->impptr(), true );
      return impptr->get_member( key.c_str() );
    }
  }
  return BObjectRef( UninitObject::create() );
}

dap::array<dap::Variable> Handles::to_variables( const BObjectRef& objref )
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
      BDictionary* dict = static_cast<BDictionary*>( impptr );
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
      ObjArray* objarr = static_cast<ObjArray*>( impptr );
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

FrameReference::FrameReference( Core::UOExecutor* uoexec, Bscript::EScriptProgram* _script,
                                size_t frameId )
    : contents()
{
  if ( frameId > uoexec->ControlStack.size() )
  {
    throw std::runtime_error( "Invalid frame id" );
  }

  std::vector<BObjectRefVec*> upperLocals2 = uoexec->upperLocals2;
  std::vector<ReturnContext> stack = uoexec->ControlStack;

  unsigned int PC;

  {
    ReturnContext rc;
    rc.PC = uoexec->PC;
    rc.ValueStackDepth = static_cast<unsigned int>( uoexec->ValueStack.size() );
    stack.push_back( rc );
  }

  upperLocals2.push_back( uoexec->Locals2 );

  auto currentFrameId = stack.size();

  while ( --currentFrameId, !stack.empty() )
  {
    ReturnContext& rc = stack.back();
    BObjectRefVec* Locals2 = upperLocals2.back();
    PC = rc.PC;
    stack.pop_back();
    upperLocals2.pop_back();

    if ( frameId != currentFrameId )
    {
      continue;
    }

    size_t left = Locals2->size();

    unsigned block = _script->dbg_ins_blocks[PC];
    while ( left )
    {
      while ( left <= _script->blocks[block].parentvariables )
      {
        block = _script->blocks[block].parentblockidx;
      }
      const EPDbgBlock& progblock = _script->blocks[block];
      size_t varidx = left - 1 - progblock.parentvariables;
      left--;
      contents[progblock.localvarnames[varidx]] = &( *Locals2 )[left];
    }
  }
}

GlobalReference::GlobalReference( Core::UOExecutor* uoexec, Bscript::EScriptProgram* _script )
    : contents()
{
  BObjectRefVec::iterator itr = uoexec->Globals2->begin(), end = uoexec->Globals2->end();

  for ( unsigned idx = 0; itr != end; ++itr, ++idx )
  {
    if ( _script->globalvarnames.size() > idx )
    {
      contents[_script->globalvarnames[idx]] = &( *itr );
    }
  }
}
}  // namespace Pol::Network::DAP
