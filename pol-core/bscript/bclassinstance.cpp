#include "bclassinstance.h"

#include "berror.h"
#include "bobject.h"
#include "clib/clib.h"
#include "clib/stlutil.h"
#include "objmembers.h"
#include "objmethods.h"

namespace Pol::Bscript
{
BClassInstance::BClassInstance( ref_ptr<EScriptProgram> program, int index,
                                std::shared_ptr<ValueStackCont> globals )
    : BStruct( OTClassInstance ), prog_( program ), index_( index ), globals( std::move( globals ) )
{
  passert( index_ < prog_->class_descriptors.size() );
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

  return new BFunctionRef( prog_, method_itr->second.function_reference_index, globals,
                           ValueStackCont{} );
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
  passert_always_r( false, "BClassInstance::copy() should never be called" );
  return nullptr;
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
    const auto funcref_index =
        prog_->class_descriptors.at( index_ ).constructor_function_reference_index;

    return BObjectRef( new BFunctionRef( prog_, funcref_index, globals, ValueStackCont{} ) );
  }

  return base::get_member_id( id );
}

std::string BClassInstance::getStringRep() const
{
  auto class_name = prog_->symbols.array() + prog_->class_descriptors[index_].name_offset;
  return fmt::format( "<class {}>", class_name );
}

BClassInstanceRef::BClassInstanceRef( BClassInstance* inst )
    : BObjectImp( BObjectType::OTClassInstanceRef ), class_instance_( inst )
{
}

size_t BClassInstanceRef::sizeEstimate() const
{
  return sizeof( BClassInstanceRef ) + class_instance_->sizeEstimate();
}

const char* BClassInstanceRef::typeOf() const
{
  return "ClassInstanceRef";
}

u8 BClassInstanceRef::typeOfInt() const
{
  return OTClassInstanceRef;
}

BObjectImp* BClassInstanceRef::copy() const
{
  return new BClassInstanceRef( class_instance_.get() );
}

bool BClassInstanceRef::isTrue() const
{
  return true;
}

BObjectImp* BClassInstanceRef::call_method( const char* methodname, Executor& ex )
{
  return class_instance_->call_method( methodname, ex );
}

BObjectImp* BClassInstanceRef::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
  return class_instance_->call_method_id( id, ex, forcebuiltin );
}

BObjectRef BClassInstanceRef::get_member_id( const int id )
{
  return class_instance_->get_member_id( id );
}

std::string BClassInstanceRef::getStringRep() const
{
  return class_instance_->getStringRep();
}

ContIterator* BClassInstanceRef::createIterator( BObject* pIterVal )
{
  return class_instance_->createIterator( pIterVal );
}

BObjectRef BClassInstanceRef::OperSubscript( const BObject& obj )
{
  return class_instance_->OperSubscript( obj );
}

BObjectRef BClassInstanceRef::set_member( const char* membername, BObjectImp* value, bool copy )
{
  return class_instance_->set_member( membername, value, copy );
}

BObjectRef BClassInstanceRef::get_member( const char* membername )
{
  return class_instance_->get_member( membername );
}

BObjectRef BClassInstanceRef::operDotPlus( const char* name )
{
  return class_instance_->operDotPlus( name );
}

BObjectRef BClassInstanceRef::operDotMinus( const char* name )
{
  return class_instance_->operDotMinus( name );
}

BObjectRef BClassInstanceRef::operDotQMark( const char* name )
{
  return class_instance_->operDotQMark( name );
}

BObjectImp* BClassInstanceRef::array_assign( BObjectImp* idx, BObjectImp* target, bool copy )
{
  return class_instance_->array_assign( idx, target, copy );
}
}  // namespace Pol::Bscript
