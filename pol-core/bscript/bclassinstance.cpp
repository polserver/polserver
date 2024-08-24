#include "bclassinstance.h"

#include "berror.h"
#include "bobject.h"
#include "objmembers.h"
#include "objmethods.h"

namespace Pol::Bscript
{
BClassInstance::BClassInstance( ref_ptr<EScriptProgram> program, int index,
                                std::shared_ptr<ValueStackCont> globals )
    : BStruct( OTClassInstance ), prog_( program ), index_( index ), globals( std::move( globals ) )
{
}

BClassInstance::BClassInstance( const BClassInstance& B ) : BStruct( B, OTClassInstance )
{
  prog_ = B.prog_;
  index_ = B.index_;
  globals = B.globals;
}

size_t BClassInstance::sizeEstimate() const
{
  return sizeof( BClassInstance );
}

ref_ptr<EScriptProgram> BClassInstance::prog() const
{
  return prog_;
}

unsigned BClassInstance::index() const
{
  return index_;
}

BFunctionRef* BClassInstance::makeMethod( const char* method_name )
{
  const auto& methods = prog_->class_descriptors[index_].methods;
  auto method_itr =
      std::find_if( methods.begin(), methods.end(),
                    [&]( const auto& it )
                    {
                      if ( it.first < prog_->symbols.length() )
                      {
                        return stricmp( method_name, prog_->symbols.array() + it.first ) == 0;
                      }
                      return false;
                    } );

  if ( method_itr == methods.end() )
    return nullptr;

  const auto& funcref_table_entry =
      prog_->function_references.at( method_itr->second.function_reference_index );

  // Subtract 1 from parameter_count of  so BFunctionRef::valid_call will
  // think a call is valid _without_ the `this`. The Executor adds `this`
  // after the validity check.
  //
  // Eg: `function foo(this, arg0)` (two params) -> `this.foo(arg0)` (one param)
  auto param_count = funcref_table_entry.parameter_count - 1;


  return new BFunctionRef( prog_, funcref_table_entry.address, param_count,
                           funcref_table_entry.is_variadic, globals, ValueStackCont{} );
}

void BClassInstance::packonto( std::ostream& os ) const
{
  // A class cannot be serialized
  os << "u";
}

const char* BClassInstance::typetag() const
{
  // Return the class name as the type tag.
  return prog_->symbols.array() + prog_->class_descriptors[index_].name_offset;
}

const char* BClassInstance::typeOf() const
{
  return "Class";
}

u8 BClassInstance::typeOfInt() const
{
  return OTClassInstance;
}

BObjectImp* BClassInstance::copy() const
{
  return new BClassInstance( *this );
}

bool BClassInstance::isTrue() const
{
  return true;
}

BObjectImp* BClassInstance::call_method( const char* methodname, Executor& /*ex*/ )
{
  // The Executor handles call_method/call_method_id directly, similar to
  // BFunctionRefs. The BClassInstance method functions only get called if the
  // Executor fails to handle them, which only happens if there is an error in
  // the call setup.
  return new BError( fmt::format( "Method '{}' not found in class '{}'", methodname, typetag() ) );
}

BObjectImp* BClassInstance::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
{
  auto method = getObjMethod( id );
  return call_method( method->code, ex );
}

BObjectRef BClassInstance::get_member_id( const int id )
{
  if ( id == MBR_FUNCTION )
  {
    if ( index_ < prog_->class_descriptors.size() )
    {
      const auto& constructors = prog_->class_descriptors.at( index_ ).constructors;
      if ( !constructors.empty() )
      {
        auto funcref_index = constructors.front().function_reference_index;
        if ( funcref_index < prog_->function_references.size() )
        {
          const auto& funcref_entry = prog_->function_references.at( funcref_index );

          return BObjectRef( new BFunctionRef(
              prog_, static_cast<int>( funcref_entry.address ), funcref_entry.parameter_count,
              funcref_entry.is_variadic, globals, ValueStackCont{} ) );
        }
      }
    }
  }

  return base::get_member_id( id );
}

std::string BClassInstance::getStringRep() const
{
  auto class_name = prog_->symbols.array() + prog_->class_descriptors[index_].name_offset;
  return fmt::format( "<class {}>", class_name );
}
}  // namespace Pol::Bscript