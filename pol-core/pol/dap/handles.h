#ifndef DAP_HANDLES_H
#define DAP_HANDLES_H

#include "../../bscript/bstruct.h"
#include "../../bscript/dict.h"
#include "../../bscript/objmembers.h"
#include "../../clib/logfacility.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"
#include "../polclass.h"
#include "../uoscrobj.h"
#include <array>
#include <dap/protocol.h>

namespace Pol
{
namespace Bscript
{
class EScriptProgram;
}
namespace Core
{
class UOExecutor;
}  // namespace Core
namespace Network
{
namespace DAP
{
class GlobalReference
{
public:
  GlobalReference( Core::UOExecutor* uoexec, Bscript::EScriptProgram* script );

  std::map<std::string, Bscript::BObjectRef*> contents;
};

class FrameReference
{
public:
  FrameReference( Core::UOExecutor* uoexec, Bscript::EScriptProgram* script, size_t frameId );

  std::map<std::string, Bscript::BObjectRef*> contents;
};

using VariableReference = Bscript::BObjectRef;

using Reference = std::variant<GlobalReference, FrameReference, VariableReference>;

class Handles
{
public:
  static constexpr int START_HANDLE = 1000;
  Handles();

  void reset();

  int create( const Reference& value );

  Reference* get( int handle );

  /**
   * Takes a `BObjectRef` and sets the `type` and `value` members of the `dap::Variable. Sets the
   * `variableReference` member for structured variables (eg. structs and arrays).
   */
  template <typename T>
  void set_value( const std::string& objref, T& variable );

  template <typename T>
  void set_response_details( const Bscript::BObjectRef& objref, T& variable );

  dap::array<dap::Variable> to_variables( const Bscript::BObjectRef& objref );
  Bscript::BObjectRef set_index_or_member( const Bscript::BObjectRef& objref,
                                           const std::string& key, Bscript::BObjectRef& value );

private:
  int _nextHandle;
  std::map<int, Reference> _handleMap;
};

template <typename T>
void Handles::set_value( const std::string& string_rep, T& variable )
{
  if constexpr ( std::is_same_v<T, dap::Variable> || std::is_same_v<T, dap::SetVariableResponse> )
  {
    variable.value = string_rep;
  }
  else if constexpr ( std::is_same_v<T, dap::EvaluateResponse> )
  {
    variable.result = string_rep;
  }
}

template <typename T>
void Handles::set_response_details( const Bscript::BObjectRef& objref, T& variable )
{
  auto impptr = objref->impptr();

  switch ( impptr->type() )
  {
  case Bscript::BObjectImp::BObjectType::OTApplicObj:
    variable.type = "OTApplicObj";
    set_value( impptr->getStringRep(), variable );
    variable.variablesReference = create( objref );
    break;
  case Bscript::BObjectImp::BObjectType::OTString:
    variable.type = "string";
    set_value( impptr->getStringRep(), variable );
    break;
  case Bscript::BObjectImp::BObjectType::OTDouble:
  case Bscript::BObjectImp::BObjectType::OTLong:
    variable.type = "number";
    set_value( impptr->getStringRep(), variable );
    break;
  case Bscript::BObjectImp::BObjectType::OTStruct:
  {
    auto size = static_cast<Bscript::BStruct*>( impptr )->contents().size();
    variable.type = "struct";
    variable.namedVariables = size;
    if ( size > 0 )
    {
      set_value( "struct{ ... }", variable );
      variable.variablesReference = create( objref );
    }
    else
    {
      set_value( "struct{ }", variable );
    }
    break;
  }
  case Bscript::BObjectImp::BObjectType::OTDictionary:
  {
    auto size = static_cast<Bscript::BDictionary*>( impptr )->contents().size();
    variable.type = "dictionary";
    variable.namedVariables = size;
    if ( size > 0 )
    {
      set_value( "dictionary{ ... }", variable );
      variable.variablesReference = create( objref );
    }
    else
    {
      set_value( "dictionary{ }", variable );
    }
    break;
  }
  case Bscript::BObjectImp::BObjectType::OTArray:
  {
    auto size = static_cast<Bscript::ObjArray*>( impptr )->ref_arr.size();
    variable.type = "array";
    variable.indexedVariables = size;
    if ( size > 0 )
    {
      set_value( "array{ ... }", variable );
      variable.variablesReference = create( objref );
    }
    else
    {
      set_value( "array{ }", variable );
    }
    break;
  }
  default:
    set_value( impptr->getStringRep(), variable );
    break;
  }
}

}  // namespace DAP
}  // namespace Network
}  // namespace Pol
#endif