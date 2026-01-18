#include "bclassinstance.h"

#include "berror.h"
#include "bobject.h"
#include "clib/clib.h"
#include "clib/stlutil.h"
#include "executor.h"
#include "objmembers.h"
#include "objmethods.h"

namespace Pol::Bscript
{
BClassInstance::BClassInstance( ref_ptr<EScriptProgram> program, int index,
                                std::shared_ptr<ValueStackCont> globals )
    : BStruct( OTClassInstance ),
      prog_( std::move( program ) ),
      index_( index ),
      globals( std::move( globals ) )
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
  return base::sizeEstimate() + Clib::memsize( constructors_called ) +
         sizeof( ref_ptr<EScriptProgram> ) + sizeof( unsigned int ) +
         sizeof( std::shared_ptr<ValueStackCont> );
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

BObjectImp* BClassInstance::call_method( const char* method_name, Executor& ex )
{
  BFunctionRef* funcr = nullptr;

  BObjectImp* callee{ nullptr };
  // Prefer members over class methods by checking contents first.
  auto member_itr = contents().find( method_name );

  if ( member_itr != contents().end() )
  {
    // If the member exists and is NOT a function reference, we will still try
    // to "call" it. This is _intentional_, and will result in a runtime
    // BError. This is similar to `var foo := 3; print(foo.bar());`, resulting
    // in a "Method 'bar' not found" error.
    callee = member_itr->second.get()->impptr();

    funcr = member_itr->second.get()->impptr_if<BFunctionRef>();
  }
  else
  {
    // Have we already looked up this method?
    Executor::ClassMethodKey key{ ex.prog_, index(), method_name };
    auto cache_itr = ex.class_methods.find( key );
    if ( cache_itr != ex.class_methods.end() )
    {
      // Switch the callee to the function reference: if the
      // funcr->validCall fails, we will go into the funcref
      // ins_call_method, giving the error about invalid parameter counts.
      funcr = cache_itr->second->impptr_if<BFunctionRef>();
      callee = funcr;
      method_name = getObjMethod( MTH_CALL_METHOD )->code;
    }
    else
    {
      // Does the class define this method?
      funcr = makeMethod( method_name );

      if ( funcr != nullptr )
      {
        // Cache the method for future lookups
        ex.class_methods[key] = BObjectRef( funcr );

        // Switch the callee to the function reference.
        callee = funcr;
        method_name = getObjMethod( MTH_CALL_METHOD )->code;
      }
    }
  }

  if ( funcr != nullptr )
  {
    Instruction jmp;
    int id;

    // Add `this` to the front of the argument list only for class methods,
    // skipping eg. an instance member function reference set via
    // `this.foo := @(){};`.
    if ( funcr->class_method() )
    {
      id = MTH_CALL_METHOD;
      ex.fparams.insert( ex.fparams.begin(), ex.ValueStack.back() );
    }
    else
    {
      id = MTH_CALL;
    }

    if ( funcr->validCall( id, ex, &jmp ) )
    {
      BObjectRef funcobj( funcr );  // valuestack gets modified, protect BFunctionRef
      ex.call_function_reference( funcr, nullptr, jmp );
      return BSpecialUserFuncJump::get();
    }
  }
  if ( callee )
    return callee->call_method( method_name, ex );
  return new BError( fmt::format( "Method '{}' not found in class '{}'", method_name, typetag() ) );
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
