#include "bfuncref.h"

#include "../clib/stlutil.h"
#include "bclassinstance.h"
#include "berror.h"
#include "blong.h"
#include "bobject.h"
#include "executor.h"
#include "objmethods.h"

#include <fmt/format.h>

namespace Pol::Bscript
{
BFunctionRef::BFunctionRef( ref_ptr<EScriptProgram> program, unsigned int pid,
                            unsigned function_reference_index,
                            std::weak_ptr<ValueStackCont> globals, ValueStackCont&& captures )
    : BObjectImp( OTFuncRef ),
      prog_( std::move( program ) ),
      original_pid_( pid ),
      function_reference_index_( function_reference_index ),
      globals( std::move( globals ) ),
      captures( std::move( captures ) )
{
  passert( function_reference_index_ < prog_->function_references.size() );
}

BFunctionRef::BFunctionRef( const BFunctionRef& B )
    : BFunctionRef( B.prog_, B.original_pid_, B.function_reference_index_, B.globals,
                    ValueStackCont( B.captures ) )
{
}

BObjectImp* BFunctionRef::copy() const
{
  return new BFunctionRef( *this );
}

size_t BFunctionRef::sizeEstimate() const
{
  return sizeof( BFunctionRef ) + Clib::memsize( captures );
}

bool BFunctionRef::isTrue() const
{
  return false;
}

bool BFunctionRef::operator==( const BObjectImp& /*objimp*/ ) const
{
  return false;
}

BObjectImp* BFunctionRef::selfIsObjImp( const BObjectImp& other ) const
{
  auto classinstref = dynamic_cast<const BClassInstanceRef*>( &other );
  if ( !classinstref )
    return new BLong( 0 );

  auto classinst = classinstref->instance();

  // Class index could be maxint if the function reference is not a class
  // method.
  if ( class_index() >= prog_->class_descriptors.size() )
    return new BLong( 0 );

  const auto& my_constructors = prog_->class_descriptors.at( class_index() ).constructors;
  passert( !my_constructors.empty() );
  const auto& my_descriptor = my_constructors.front();

  const auto& other_descriptors =
      classinst->prog()->class_descriptors.at( classinst->index() ).constructors;

  // An optimization for same program: since strings are never duplicated inside
  // the data section, we can just check for offset equality.
  if ( prog_ == classinst->prog() )
  {
    for ( const auto& other_descriptor : other_descriptors )
    {
      if ( my_descriptor.type_tag_offset == other_descriptor.type_tag_offset )
        return new BLong( 1 );
    }
  }
  // For different programs, we must check for string equality.
  else
  {
    auto type_tag = prog_->symbols.array() + my_descriptor.type_tag_offset;
    for ( const auto& other_descriptor : other_descriptors )
    {
      auto other_type_tag = classinst->prog()->symbols.array() + other_descriptor.type_tag_offset;
      if ( strcmp( type_tag, other_type_tag ) == 0 )
        return new BLong( 1 );
    }
  }

  return new BLong( 0 );
}

std::string BFunctionRef::getStringRep() const
{
  return "FunctionObject";
}

BObjectImp* BFunctionRef::call_method( const char* methodname, Executor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return call_method_id( objmethod->id, ex );
  return nullptr;
}

bool BFunctionRef::validCall( const int id, Executor& ex, Instruction* inst ) const
{
  auto passed_args = static_cast<int>( ex.numParams() );

  auto [expected_min_args, expected_max_args] = expected_args();

  if ( id != MTH_CALL && id != MTH_NEW && id != MTH_CALL_METHOD )
    return false;

  if ( id == MTH_NEW && !constructor() )
    return false;

  if ( passed_args < expected_min_args || ( passed_args > expected_max_args && !variadic() ) )
    return false;

  inst->func = &Executor::ins_nop;

  if ( passed_args >= expected_max_args )
  {
    inst->token.lval = pc();
  }
  else
  {
    auto default_parameter_address_index = expected_max_args - passed_args - 1;
    passert( default_parameter_address_index >= 0 &&
             default_parameter_address_index <
                 static_cast<int>( default_parameter_addresses().size() ) );
    inst->token.lval = default_parameter_addresses().at( default_parameter_address_index );
  }

  return true;
}

bool BFunctionRef::validCall( const char* methodname, Executor& ex, Instruction* inst ) const
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod == nullptr )
    return false;
  return validCall( objmethod->id, ex, inst );
}

int BFunctionRef::numParams() const
{
  return prog_->function_references[function_reference_index_].parameter_count;
}

unsigned BFunctionRef::pc() const
{
  return prog_->function_references[function_reference_index_].address;
}

bool BFunctionRef::variadic() const
{
  return prog_->function_references[function_reference_index_].is_variadic;
}

ref_ptr<EScriptProgram> BFunctionRef::prog() const
{
  return prog_;
}

unsigned int BFunctionRef::pid() const
{
  return original_pid_;
}

unsigned BFunctionRef::class_index() const
{
  return prog_->function_references[function_reference_index_].class_index;
}

bool BFunctionRef::constructor() const
{
  return prog_->function_references[function_reference_index_].is_constructor;
}

bool BFunctionRef::class_method() const
{
  return prog_->function_references[function_reference_index_].class_index <
         std::numeric_limits<unsigned>::max();
}

const std::vector<unsigned>& BFunctionRef::default_parameter_addresses() const
{
  return prog_->function_references[function_reference_index_].default_parameter_addresses;
}

int BFunctionRef::default_parameter_count() const
{
  return static_cast<int>( default_parameter_addresses().size() );
}

std::pair<int, int> BFunctionRef::expected_args() const
{
  auto expected_min_args = numParams() - default_parameter_count();  // remove default parameters

  return { expected_min_args, expected_min_args + default_parameter_count() };
}

BObjectImp* BFunctionRef::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
{
  bool adjust_for_this = false;

  // These are only entered if `ins_call_method_id` did _not_ do the call jump.
  switch ( id )
  {
  case MTH_NEW:
    if ( !constructor() )
      return new BError( "Function is not a constructor" );
    // intentional fallthrough
  case MTH_CALL_METHOD:
    adjust_for_this = true;
    // intentional fallthrough
  case MTH_CALL:
  {
    auto [expected_min_args, expected_max_args] = expected_args();
    auto param_count = ex.numParams();

    if ( adjust_for_this )
    {
      --expected_min_args;
      --expected_max_args;
      --param_count;
    }

    auto expected_arg_string = variadic() ? fmt::format( "{}+", expected_min_args )
                               : ( expected_min_args == expected_max_args )
                                   ? std::to_string( expected_min_args )
                                   : fmt::format( "{}-{}", expected_min_args, expected_max_args );

    return new BError( fmt::format( "Invalid argument count: expected {}, got {}",
                                    expected_arg_string, param_count ) );
  }
  default:
    return nullptr;
  }
}
}  // namespace Pol::Bscript
