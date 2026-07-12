#include "barray.h"

#include "clib/logfacility.h"
#include "clib/random.h"
#include "clib/stlutil.h"

#include "bdouble.h"
#include "berror.h"
#include "blong.h"
#include "bobject.h"
#include "bstring.h"
#include "buninit.h"
#include "executor.h"
#include "executor.inl.h"
#include "objmembers.h"
#include "objmethods.h"
#include "str.h"

namespace Pol::Bscript
{
using namespace fmt::literals;

ObjArray::ObjArray() : BObjectImp( OTArray ), name_arr(), ref_arr() {}

ObjArray::ObjArray( BObjectType type ) : BObjectImp( type ), name_arr(), ref_arr() {}

ObjArray::ObjArray( const ObjArray& copyfrom )
    : BObjectImp( copyfrom.type() ), name_arr( copyfrom.name_arr ), ref_arr( copyfrom.ref_arr )
{
  deepcopy();
}

void ObjArray::deepcopy()
{
  for ( auto& elem : ref_arr )
  {
    if ( elem.get() )
    {
      /*
          NOTE: all BObjectRefs in an ObjArray reference BNamedObjects not BObjects
          HMMM, can this BNamedObject get destructed before we're done with it?
          No, we're making a copy, leaving the original be.
          (SO, bno's refcount should be >1 here)
          */

      BObject* bo = elem.get();
      elem.set( new BObject( bo->impptr()->copy() ) );
    }
  }
}

BObjectImp* ObjArray::copy() const
{
  auto nobj = new ObjArray( *this );
  return nobj;
}

size_t ObjArray::sizeEstimate() const
{
  size_t size = sizeof( ObjArray );
  size += Clib::memsize( ref_arr );
  for ( const auto& elem : ref_arr )
  {
    size += elem.sizeEstimate();
  }
  size += Clib::memsize( name_arr );
  for ( const auto& elem : name_arr )
  {
    size += elem.capacity();
  }
  return size;
}

/**
 * Equality for arrays:
 * if the other guy is an array, check each element
 * otherwise not equal.
 * note that struct names aren't checked.
 *
 * @todo check structure names too?
 */
bool ObjArray::operator==( const BObjectImp& imp ) const
{
  if ( !imp.isa( OTArray ) )
    return false;

  const ObjArray& thatarr = static_cast<const ObjArray&>( imp );
  if ( thatarr.ref_arr.size() != ref_arr.size() )
    return false;

  for ( unsigned i = 0; i < ref_arr.size(); ++i )
  {
    const BObjectRef& thisref = ref_arr[i];
    const BObjectRef& thatref = thatarr.ref_arr[i];

    const BObject* thisobj = thisref.get();
    const BObject* thatobj = thatref.get();
    if ( thisobj != nullptr && thatobj != nullptr )
    {
      const BObjectImp& thisimp = thisobj->impref();
      const BObjectImp& thatimp = thatobj->impref();

      if ( thisimp == thatimp )
        continue;
      return false;
    }
    if ( thisobj == nullptr && thatobj == nullptr )
    {
      continue;
    }

    return false;
  }
  return true;
}

const BObjectImp* ObjArray::imp_at( unsigned index /* 1-based */ ) const
{
  assert( index > 0 );
  if ( index > ref_arr.size() )
    return nullptr;
  const BObjectRef& ref = ref_arr[index - 1];
  if ( ref.get() == nullptr )
    return nullptr;
  return ref.get()->impptr();
}

BObjectImp* ObjArray::array_assign( BObjectImp* idx, BObjectImp* target, bool copy )
{
  if ( idx->isa( OTLong ) )
  {
    BLong& lng = (BLong&)*idx;

    unsigned index = (unsigned)lng.value();
    if ( index > ref_arr.size() )
      ref_arr.resize( index );
    else if ( index <= 0 )
      return new BError( "Array index out of bounds" );

    BObjectRef& ref = ref_arr[index - 1];
    BObject* refobj = ref.get();

    BObjectImp* new_target = copy ? target->copy() : target;

    if ( refobj != nullptr )
    {
      refobj->setimp( new_target );
    }
    else
    {
      ref.set( new BObject( new_target ) );
    }
    return ref->impptr();
  }

  return UninitObject::create();
}

void ObjArray::operInsertInto( BObject& /*obj*/, const BObjectImp& objimp )
{
  ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}

BObjectImp* ObjArray::selfPlusObj( const BObjectImp& objimp ) const
{
  std::unique_ptr<ObjArray> result( new ObjArray( *this ) );
  result->ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
  return result.release();
}
BObjectImp* ObjArray::selfPlusObj( const BLong& objimp ) const
{
  std::unique_ptr<ObjArray> result( new ObjArray( *this ) );
  result->ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
  return result.release();
}
BObjectImp* ObjArray::selfPlusObj( const Double& objimp ) const
{
  std::unique_ptr<ObjArray> result( new ObjArray( *this ) );
  result->ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
  return result.release();
}
BObjectImp* ObjArray::selfPlusObj( const String& objimp ) const
{
  std::unique_ptr<ObjArray> result( new ObjArray( *this ) );
  result->ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
  return result.release();
}

BObjectImp* ObjArray::selfPlusObj( const ObjArray& objimp ) const
{
  std::unique_ptr<ObjArray> result( new ObjArray( *this ) );

  for ( const auto& elem : objimp.ref_arr )
  {
    if ( elem.get() )
    {
      /*
          NOTE: all BObjectRefs in an ObjArray reference BNamedObjects not BObjects
          HMMM, can this BNamedObject get destructed before we're done with it?
          No, we're making a copy, leaving the original be.
          (SO, bno's refcount should be >1 here)
          */
      BObject* bo = elem.get();

      result->ref_arr.push_back( BObjectRef( new BObject( ( *bo )->copy() ) ) );
    }
    else
    {
      result->ref_arr.emplace_back();
    }
  }
  return result.release();
}

BObjectImp* ObjArray::selfPlusObjImp( const BObjectImp& other ) const
{
  return other.selfPlusObj( *this );
}
void ObjArray::selfPlusObjImp( BObjectImp& objimp, BObject& obj )
{
  objimp.selfPlusObj( *this, obj );
}
void ObjArray::selfPlusObj( BObjectImp& objimp, BObject& /*obj*/ )
{
  ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}
void ObjArray::selfPlusObj( BLong& objimp, BObject& /*obj*/ )
{
  ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}
void ObjArray::selfPlusObj( Double& objimp, BObject& /*obj*/ )
{
  ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}
void ObjArray::selfPlusObj( String& objimp, BObject& /*obj*/ )
{
  ref_arr.push_back( BObjectRef( new BObject( objimp.copy() ) ) );
}
void ObjArray::selfPlusObj( ObjArray& objimp, BObject& /*obj*/ )
{
  for ( const auto& itr : objimp.ref_arr )
  {
    if ( itr.get() )
    {
      /*
          NOTE: all BObjectRefs in an ObjArray reference BNamedObjects not BObjects
          HMMM, can this BNamedObject get destructed before we're done with it?
          No, we're making a copy, leaving the original be.
          (SO, bno's refcount should be >1 here)
          */
      BObject* bo = itr.get();

      ref_arr.push_back( BObjectRef( new BObject( ( *bo )->copy() ) ) );
    }
    else
    {
      ref_arr.emplace_back();
    }
  }
}

BObjectRef ObjArray::OperMultiSubscript( std::stack<BObjectRef>& indices )
{
  BObjectRef start_ref = indices.top();
  indices.pop();
  BObjectRef length_ref = indices.top();
  indices.pop();

  BObject& length_obj = *length_ref;
  BObject& start_obj = *start_ref;

  BObjectImp& length = length_obj.impref();
  BObjectImp& start = start_obj.impref();

  // first deal with the start position.
  // return BObjectRef(new BError( "Subscript out of range" ));

  unsigned index;
  if ( start.isa( OTLong ) )
  {
    BLong& lng = (BLong&)start;
    index = (unsigned)lng.value();
    if ( index == 0 || index > ref_arr.size() )
      return BObjectRef( new BError( "Array start index out of bounds" ) );
  }
  else
    return BObjectRef( copy() );

  // now the end index

  unsigned end;
  if ( length.isa( OTLong ) )
  {
    BLong& lng = (BLong&)length;
    end = (unsigned)lng.value();
    if ( end == 0 || end > ref_arr.size() )
      return BObjectRef( new BError( "Array end index out of bounds" ) );
  }
  else
    return BObjectRef( copy() );

  auto str = new ObjArray();


  // std::unique_ptr<ObjArray> result (new ObjArray());
  unsigned i = 0;
  for ( const auto& itr : ref_arr )
  {
    if ( ++i < index )
      continue;
    if ( i > end )
      break;
    if ( itr.get() )
    {
      BObject* bo = itr.get();
      str->ref_arr.push_back( BObjectRef( new BObject( ( *bo )->copy() ) ) );
    }
    else
    {
      str->ref_arr.emplace_back();
    }
  }
  /*
  for (unsigned i = index; i < end; i++) {
  BObject *bo = ref_arr[i];
  if (bo != 0)
  str->ref_arr.push_back( BObjectRef( new BObject( (*bo)->copy() ) ) );
  else
  result->ref_arr.push_back( BObjectRef() );
  } */
  //  return result.release();
  return BObjectRef( str );
}

BObjectRef ObjArray::OperSubscript( const BObject& rightobj )
{
  const BObjectImp& right = rightobj.impref();
  if ( right.isa( OTLong ) )  // vector
  {
    BLong& lng = (BLong&)right;

    unsigned index = (unsigned)lng.value();
    if ( index > ref_arr.size() )
    {
      return BObjectRef( new BError( "Array index out of bounds" ) );
    }
    if ( index <= 0 )
      return BObjectRef( new BError( "Array index out of bounds" ) );

    BObjectRef& ref = ref_arr[index - 1];
    if ( ref.get() == nullptr )
      ref.set( new BObject( UninitObject::create() ) );
    return ref;
  }
  if ( right.isa( OTString ) )
  {
    // TODO: search for named variables (structure members)
    return BObjectRef( copy() );
  }

  // TODO: crap out
  return BObjectRef( copy() );
}

BObjectRef ObjArray::get_member( const char* membername )
{
  int i = 0;
  for ( const_name_iterator itr = name_arr.begin(), end = name_arr.end(); itr != end; ++itr, ++i )
  {
    const std::string& name = ( *itr );
    if ( stricmp( name.c_str(), membername ) == 0 )
    {
      return ref_arr[i];
    }
  }

  return BObjectRef( UninitObject::create() );
}

BObjectRef ObjArray::set_member( const char* membername, BObjectImp* valueimp, bool copy )
{
  int i = 0;
  for ( const_name_iterator itr = name_arr.begin(), end = name_arr.end(); itr != end; ++itr, ++i )
  {
    const std::string& name = ( *itr );
    if ( stricmp( name.c_str(), membername ) == 0 )
    {
      BObjectImp* target = copy ? valueimp->copy() : valueimp;
      ref_arr[i].get()->setimp( target );
      return ref_arr[i];
    }
  }
  return BObjectRef( UninitObject::create() );
}

BObjectRef ObjArray::operDotPlus( const char* name )
{
  for ( auto& elem : name_arr )
  {
    if ( stricmp( name, elem.c_str() ) == 0 )
    {
      return BObjectRef( new BError( "Member already exists" ) );
    }
  }
  name_arr.emplace_back( name );
  auto pnewobj = new BObject( UninitObject::create() );
  ref_arr.emplace_back( pnewobj );
  return BObjectRef( pnewobj );
}

void ObjArray::addElement( BObjectImp* imp )
{
  ref_arr.push_back( BObjectRef( new BObject( imp ) ) );
}

std::string ObjArray::getStringRep() const
{
  std::string rep{ "{ " };
  bool any = false;
  for ( const auto& elem : ref_arr )
  {
    if ( any )
      rep += ", ";
    else
      any = true;

    BObject* bo = elem.get();

    if ( bo != nullptr )
      rep += bo->impptr()->getStringRep();
  }
  rep += " }";

  return rep;
}

long ObjArray::contains( const BObjectImp& imp ) const
{
  for ( auto itr = ref_arr.begin(), end = ref_arr.end(); itr != end; ++itr )
  {
    if ( itr->get() )
    {
      BObject* bo = ( itr->get() );
      if ( bo == nullptr )
      {
        INFO_PRINTLN( "{} - '{} in array{{}}' check. Invalid data at index {}",
                      Clib::scripts_thread_script, imp, ( itr - ref_arr.begin() ) + 1 );
        continue;
      }
      if ( *( bo->impptr() ) == imp )
      {
        return ( static_cast<long>( ( itr - ref_arr.begin() ) + 1 ) );
      }
    }
  }
  return 0;
}

class objref_cmp
{
public:
  bool operator()( const BObjectRef& x1, const BObjectRef& x2 ) const
  {
    const BObject* b1 = x1.get();
    const BObject* b2 = x2.get();
    if ( b1 == nullptr || b2 == nullptr )
      return ( &x1 < &x2 );

    const BObject& r1 = *b1;
    const BObject& r2 = *b2;
    return ( r1 < r2 );
  }
};

BObjectImp* ObjArray::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
{
  switch ( id )
  {
  case MTH_SIZE:
    if ( ex.numParams() == 0 )
      return new BLong( static_cast<int>( ref_arr.size() ) );
    else
      return new BError( "array.size() doesn't take parameters." );

  case MTH_ERASE:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 1 )
      {
        int idx;
        if ( ex.getParam( 0, idx, 1, static_cast<int>( ref_arr.size() ) ) )  // 1-based index
        {
          ref_arr.erase( ref_arr.begin() + idx - 1 );
          return new BLong( 1 );
        }

        return nullptr;
      }
      return new BError( "array.erase(index) requires a parameter." );
    }
    break;
  case MTH_EXISTS:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 1 )
      {
        int idx;
        if ( ex.getParam( 0, idx ) && idx >= 0 )
        {
          bool exists = ( idx <= (int)ref_arr.size() );
          return new BLong( exists ? 1 : 0 );
        }

        return new BError( "Invalid parameter type" );
      }
      return new BError( "array.exists(index) requires a parameter." );
    }
    break;
  case MTH_INSERT:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 2 )
      {
        int idx;
        BObjectImp* imp = ex.getParamImp( 1 );
        if ( ex.getParam( 0, idx, 1, static_cast<int>( ref_arr.size() + 1 ) ) &&
             imp != nullptr )  // 1-based index
        {
          --idx;
          BObjectRef tmp;
          ref_arr.insert( ref_arr.begin() + idx, tmp );
          BObjectRef& ref = ref_arr[idx];
          ref.set( new BObject( imp->copy() ) );
        }
        else
        {
          return new BError( "Invalid parameter type" );
        }
      }
      else
        return new BError( "array.insert(index,value) requires two parameters." );
    }
    break;
  case MTH_SHRINK:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 1 )
      {
        int idx;
        if ( ex.getParam( 0, idx, 0, static_cast<int>( ref_arr.size() ) ) )  // 1-based index
        {
          ref_arr.erase( ref_arr.begin() + idx, ref_arr.end() );
          return new BLong( 1 );
        }

        return new BError( "Invalid parameter type" );
      }
      return new BError( "array.shrink(nelems) requires a parameter." );
    }
    break;
  case MTH_APPEND:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 1 )
      {
        BObjectImp* imp = ex.getParamImp( 0 );
        if ( imp )
        {
          ref_arr.push_back( BObjectRef( new BObject( imp->copy() ) ) );

          return new BLong( 1 );
        }

        return new BError( "Invalid parameter type" );
      }
      return new BError( "array.append(value) requires a parameter." );
    }
    break;
  case MTH_REVERSE:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 0 )
      {
        reverse( ref_arr.begin(), ref_arr.end() );
        return new BLong( 1 );
      }
      return new BError( "array.reverse() doesn't take parameters." );
    }
    break;
  case MTH_SORT:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 0 )
      {
        sort( ref_arr.begin(), ref_arr.end(), objref_cmp() );
        return new BLong( 1 );
      }
      if ( ex.numParams() == 1 )
      {
        int sub_index;
        if ( !ex.getParam( 0, sub_index ) )
          return new BError( "Invalid parameter type" );
        if ( sub_index < 1 )
          return new BError( "Invalid sub_index value" );
        for ( const auto& ref : ref_arr )
        {
          if ( ref.get() == nullptr || !ref.get()->isa( OTArray ) )
            return new BError( "Invalid array" );
          auto sub_arr = ref.get()->impptr<ObjArray>();
          if ( sub_arr->ref_arr.size() < static_cast<size_t>( sub_index ) )
            return new BError( "Subindex to large" );
        }
        sort( ref_arr.begin(), ref_arr.end(),
              [=]( const BObjectRef& x1, const BObjectRef& x2 ) -> bool
              {
                auto sub_arr1 = x1.get()->impptr<ObjArray>();
                auto sub_arr2 = x2.get()->impptr<ObjArray>();
                auto sub1 = sub_arr1->ref_arr[sub_index - 1];
                auto sub2 = sub_arr2->ref_arr[sub_index - 1];
                const BObject* b1 = sub1.get();
                const BObject* b2 = sub2.get();
                if ( b1 == nullptr || b2 == nullptr )
                  return ( &x1 < &x2 );
                return ( *b1 < *b2 );
              } );
        return new BLong( 1 );
      }
      return new BError( "array.sort(sub_index=0) takes at most one parameter." );
    }
    break;
  case MTH_RANDOMENTRY:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() == 0 )
      {
        if ( !ref_arr.empty() )
        {
          const BObjectRef& ref =
              ref_arr[Clib::random_int( static_cast<int>( ref_arr.size() ) - 1 )];
          if ( ref.get() == nullptr )
            return nullptr;
          return ref.get()->impptr();
        }
      }
      else
        return new BError( "array.randomentry() doesn't take parameters." );
    }
    break;
  case MTH_FILTER:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() < 1 )
        return new BError( "Invalid parameter type" );

      BObjectImp* param0 = ex.getParamImp( 0, BObjectType::OTFuncRef );

      if ( !param0 )
        return new BError( "Invalid parameter type" );

      // The filter callback allows optional arguments, so no need to check the
      // number of arguments for the passed function reference. Arguments passed
      // will be shrunk and expanded (with uninit) as needed.

      // If nothing to filter, return an empty array, since nothing to call the function with.
      if ( ref_arr.empty() )
        return new ObjArray();

      // Arguments for user function call.
      // - the element
      // - the index of the element
      // - the array itself
      BObjectRefVec args;
      args.push_back( ref_arr.front() );
      args.push_back( BObjectRef( new BLong( 1 ) ) );
      args.emplace_back( this );

      // The ContinuationCallback receives three arguments:
      //
      // - `Executor&`
      // - `BContinuation* continuation`: The continuation, with methods to handle
      //   the continuation (call the function again; finalize)
      // - `BObjectRef result`: The result of the user function call specified in
      //   `makeContinuation`.
      //
      // We pass to the lambda a reference to the element in case the user
      // function modifies ref_arr.
      //
      // Returns a `BObjectImp`:
      // - Call the user function again by returning the same continuation via
      //   `ex.withContinuation`.
      // - Return something else (in this case, the filtered array) to provide
      //   that value back to the script.
      auto callback = [elementRef = args[0], processed = 1, thisArray = args[2],
                       filteredRef = BObjectRef( new ObjArray ),
                       initialSize = static_cast<int>( ref_arr.size() )](
                          Executor& ex, BContinuation* continuation,
                          BObjectRef result ) mutable -> BObjectImp*
      {
        auto filtered = filteredRef->impptr<ObjArray>();

        // Do something with result.
        // If the result is true, add it to the filtered array.
        if ( result->isTrue() )
        {
          filtered->ref_arr.emplace_back( elementRef->impptr() );
        }

        // If thisArray was modified for some reason to no longer be an array,
        // return the filtered array.
        if ( !thisArray->isa( OTArray ) )
          return filtered;

        const auto& ref_arr = thisArray->impptr<ObjArray>()->ref_arr;

        // If the processed index is the last element, return the filtered
        // array. Also check if the processed index is greater than the initial
        // size of the array, as the user function may have modified the array.
        if ( processed >= initialSize || processed >= static_cast<int>( ref_arr.size() ) )
        {
          return filtered;
        }
        // Otherwise, increment the processed index and call the function again.

        // Increment the processed counter.
        ++processed;

        BObjectRefVec args;
        args.push_back( ref_arr[processed - 1] );
        args.push_back( BObjectRef( new BObject( new BLong( processed ) ) ) );
        args.push_back( thisArray );

        elementRef = args[0];

        // Return this continuation with the new arguments.
        return ex.withContinuation( continuation, std::move( args ) );
      };

      // Create a new continuation for a user function call.
      return ex.makeContinuation( BObjectRef( new BObject( param0 ) ), std::move( callback ),
                                  std::move( args ) );
    }
    break;

  case MTH_MAP:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() < 1 )
        return new BError( "Invalid parameter type" );

      BObjectImp* param0 = ex.getParamImp( 0, BObjectType::OTFuncRef );

      if ( !param0 )
        return new BError( "Invalid parameter type" );

      if ( ref_arr.empty() )
        return new ObjArray();

      // Arguments for user function call.
      // - the element
      // - the index of the element
      // - the array itself
      BObjectRefVec args;
      args.push_back( ref_arr.front() );
      args.push_back( BObjectRef( new BLong( 1 ) ) );
      args.emplace_back( this );

      auto callback = [elementRef = args[0], processed = 1, thisArray = args[2],
                       mappedRef = BObjectRef( new ObjArray ),
                       initialSize = static_cast<int>( ref_arr.size() )](
                          Executor& ex, BContinuation* continuation,
                          BObjectRef result ) mutable -> BObjectImp*
      {
        auto mapped = mappedRef->impptr<ObjArray>();

        mapped->ref_arr.emplace_back( result->impptr() );

        if ( !thisArray->isa( OTArray ) )
          return mapped;

        const auto& ref_arr = thisArray->impptr<ObjArray>()->ref_arr;

        if ( processed >= initialSize || processed >= static_cast<int>( ref_arr.size() ) )
        {
          return mapped;
        }

        // Increment the processed counter.
        ++processed;

        BObjectRefVec args;
        args.push_back( ref_arr[processed - 1] );
        args.push_back( BObjectRef( new BObject( new BLong( processed ) ) ) );
        args.push_back( thisArray );

        elementRef = args[0];

        return ex.withContinuation( continuation, std::move( args ) );
      };

      return ex.makeContinuation( BObjectRef( new BObject( param0 ) ), std::move( callback ),
                                  std::move( args ) );
    }
    break;

  case MTH_REDUCE:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() < 1 )
        return new BError( "Invalid parameter type" );

      BObjectImp* param0 = ex.getParamImp( 0, BObjectType::OTFuncRef );

      if ( !param0 )
        return new BError( "Invalid parameter type" );

      BObjectImp* accumulator;
      int processed;

      // If an initial accumulator value was passed in, use it. Otherwise, use
      // the first element of the array, erroring if the array is empty.
      if ( ex.numParams() > 1 )
      {
        accumulator = ex.getParamImp( 1 );
        processed = 1;
      }
      else if ( ref_arr.empty() )
      {
        return new BError( "Reduce of empty array with no initial value" );
      }
      else
      {
        accumulator = ref_arr[0]->impptr();
        processed = 2;
      }

      // Return the accumulator if there is no more to process, eg:
      // {}.reduce(@{}, "accum") or {"accum"}.reduce(@{})
      if ( processed > static_cast<int>( ref_arr.size() ) )
      {
        return accumulator;
      }

      // Arguments for user function call.
      // - accumulator
      // - current value
      // - current index
      // - the array itself
      BObjectRefVec args;
      args.emplace_back( accumulator );
      args.emplace_back( ref_arr[processed - 1] );
      args.push_back( BObjectRef( new BLong( processed ) ) );
      args.emplace_back( this );

      auto callback = [thisArray = args[3], processed = processed,
                       initialSize = static_cast<int>( ref_arr.size() )](
                          Executor& ex, BContinuation* continuation,
                          BObjectRef result /* accumulator */ ) mutable -> BObjectImp*
      {
        if ( !thisArray->isa( OTArray ) )
          return result->impptr();

        const auto& ref_arr = thisArray->impptr<ObjArray>()->ref_arr;

        if ( processed >= initialSize || processed >= static_cast<int>( ref_arr.size() ) )
        {
          return result->impptr();
        }

        ++processed;

        BObjectRefVec args;
        args.push_back( result );
        args.push_back( ref_arr[processed - 1] );
        args.push_back( BObjectRef( new BObject( new BLong( processed ) ) ) );
        args.push_back( thisArray );

        return ex.withContinuation( continuation, std::move( args ) );
      };

      return ex.makeContinuation( BObjectRef( new BObject( param0 ) ), std::move( callback ),
                                  std::move( args ) );
    }
    break;

  case MTH_FIND:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() < 1 )
        return new BError( "Invalid parameter type" );

      BObjectImp* param0 = ex.getParamImp( 0, BObjectType::OTFuncRef );

      if ( !param0 )
        return new BError( "Invalid parameter type" );

      if ( ref_arr.empty() )
        return new ObjArray();

      // Arguments for user function call.
      // - the element
      // - the index of the element
      // - the array itself
      BObjectRefVec args;
      args.push_back( ref_arr.front() );
      args.push_back( BObjectRef( new BLong( 1 ) ) );
      args.emplace_back( this );

      auto callback = [elementRef = args[0], processed = 1, thisArray = args[2],
                       initialSize = static_cast<int>( ref_arr.size() )](
                          Executor& ex, BContinuation* continuation,
                          BObjectRef result ) mutable -> BObjectImp*
      {
        if ( result->isTrue() )
        {
          return elementRef->impptr();
        }

        if ( !thisArray->isa( OTArray ) )
          return UninitObject::create();

        const auto& ref_arr = thisArray->impptr<ObjArray>()->ref_arr;

        if ( processed >= initialSize || processed >= static_cast<int>( ref_arr.size() ) )
        {
          return UninitObject::create();
        }

        ++processed;

        BObjectRefVec args;
        args.push_back( ref_arr[processed - 1] );
        args.push_back( BObjectRef( new BObject( new BLong( processed ) ) ) );
        args.push_back( thisArray );

        elementRef = args[0];

        return ex.withContinuation( continuation, std::move( args ) );
      };

      return ex.makeContinuation( BObjectRef( new BObject( param0 ) ), std::move( callback ),
                                  std::move( args ) );
    }
    break;

  case MTH_FINDINDEX:
    if ( name_arr.empty() )
    {
      if ( ex.numParams() < 1 )
        return new BError( "Invalid parameter type" );

      BObjectImp* param0 = ex.getParamImp( 0, BObjectType::OTFuncRef );

      if ( !param0 )
        return new BError( "Invalid parameter type" );

      if ( ref_arr.empty() )
        return new BLong( 0 );

      // Arguments for user function call.
      // - the element
      // - the index of the element
      // - the array itself
      BObjectRefVec args;
      args.push_back( ref_arr.front() );
      args.push_back( BObjectRef( new BLong( 1 ) ) );
      args.emplace_back( this );

      auto callback =
          [processed = 1, thisArray = args[2], initialSize = static_cast<int>( ref_arr.size() )](
              Executor& ex, BContinuation* continuation, BObjectRef result ) mutable -> BObjectImp*
      {
        if ( result->isTrue() )
        {
          return new BLong( processed );
        }

        if ( !thisArray->isa( OTArray ) )
          return new BLong( 0 );

        const auto& ref_arr = thisArray->impptr<ObjArray>()->ref_arr;

        if ( processed >= initialSize || processed >= static_cast<int>( ref_arr.size() ) )
        {
          return new BLong( 0 );
        }

        ++processed;

        BObjectRefVec args;
        args.push_back( ref_arr[processed - 1] );
        args.push_back( BObjectRef( new BObject( new BLong( processed ) ) ) );
        args.push_back( thisArray );

        return ex.withContinuation( continuation, std::move( args ) );
      };

      return ex.makeContinuation( BObjectRef( new BObject( param0 ) ), std::move( callback ),
                                  std::move( args ) );
    }
    break;

  case MTH_CYCLE:
    if ( name_arr.empty() )
    {
      int shift_by;

      if ( ex.numParams() > 0 )
      {
        if ( !ex.getParam( 0, shift_by ) )
          return new BError( "Invalid parameter type" );
        if ( shift_by == 0 )
          return new BLong( 0 );
      }
      else
        shift_by = 1;

      if ( ref_arr.empty() || std::abs( shift_by ) > (int)ref_arr.size() )
        return new BLong( 0 );

      if ( shift_by > 0 )
        std::rotate( ref_arr.begin(), ref_arr.end() - shift_by, ref_arr.end() );
      else
        std::rotate( ref_arr.begin(), ref_arr.begin() - shift_by, ref_arr.end() );

      return new BLong( 1 );
    }
    break;

  case MTH_SORTEDINSERT:
  {
    if ( !name_arr.empty() )
      break;
    if ( ex.numParams() == 0 )
      return new BError(
          "array.sorted_insert(obj, sub_index:=0, reverse:=0) takes at least one parameter." );
    BObjectImp* imp = ex.getParamImp( 0 );
    if ( !imp )
      return new BError( "Invalid parameter type" );
    bool reverse = false;
    int sub_index = 0;
    if ( ex.numParams() >= 2 )
    {
      if ( !ex.getParam( 1, sub_index ) )
        return new BError( "Invalid parameter type" );
      if ( sub_index < 0 )
        return new BError( "Invalid sub_index value" );
    }
    if ( ex.numParams() >= 3 )
    {
      int reverseparam;
      if ( !ex.getParam( 2, reverseparam ) )
        return new BError( "Invalid parameter type" );
      reverse = reverseparam != 0;
    }
    BObjectRef item( new BObject( imp->copy() ) );
    if ( !sub_index )
    {
      if ( reverse )
      {
        ref_arr.insert( std::lower_bound( ref_arr.begin(), ref_arr.end(), item,
                                          []( const BObjectRef& x1, const BObjectRef& x2 ) -> bool
                                          {
                                            const BObject* b1 = x1.get();
                                            const BObject* b2 = x2.get();
                                            if ( b1 == nullptr || b2 == nullptr )
                                              return ( &x1 > &x2 );
                                            const BObject& r1 = *b1;
                                            const BObject& r2 = *b2;
                                            return ( r1 > r2 );
                                          } ),
                        item );
      }
      else
      {
        ref_arr.insert( std::upper_bound( ref_arr.begin(), ref_arr.end(), item, objref_cmp() ),
                        item );
      }
    }
    else
    {
      auto cmp_func = [=]( const BObjectRef& x1, const BObjectRef& x2 ) -> bool
      {
        if ( x1.get() == nullptr || !x1.get()->isa( OTArray ) )
          return false;
        if ( x2.get() == nullptr || !x2.get()->isa( OTArray ) )
          return false;
        auto sub_arr1 = x1.get()->impptr<ObjArray>();
        auto sub_arr2 = x2.get()->impptr<ObjArray>();
        if ( sub_arr1->ref_arr.size() < static_cast<size_t>( sub_index ) )
          return false;
        if ( sub_arr2->ref_arr.size() < static_cast<size_t>( sub_index ) )
          return false;
        auto sub1 = sub_arr1->ref_arr[sub_index - 1];
        auto sub2 = sub_arr2->ref_arr[sub_index - 1];
        const BObject* b1 = sub1.get();
        const BObject* b2 = sub2.get();
        if ( !reverse )
        {
          if ( b1 == nullptr || b2 == nullptr )
            return ( &x1 < &x2 );
          return ( *b1 < *b2 );
        }

        if ( b1 == nullptr || b2 == nullptr )
          return ( &x1 > &x2 );
        return ( *b1 > *b2 );
      };
      if ( reverse )
      {
        ref_arr.insert( std::lower_bound( ref_arr.begin(), ref_arr.end(), item, cmp_func ), item );
      }
      else
      {
        ref_arr.insert( std::upper_bound( ref_arr.begin(), ref_arr.end(), item, cmp_func ), item );
      }
    }
    return new BLong( 1 );
    break;
  }
  default:
    return nullptr;
  }
  return nullptr;
}

BObjectImp* ObjArray::call_method( const char* methodname, Executor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_method_id( objmethod->id, ex );
  return nullptr;
}

void ObjArray::packonto( std::string& str ) const
{
  fmt::format_to( std::back_inserter( str ), "a{}:"_cf, ref_arr.size() );
  for ( const auto& elem : ref_arr )
  {
    if ( elem.get() )
    {
      BObject* bo = elem.get();
      bo->impptr()->packonto( str );
    }
    else
    {
      str += "x";
    }
  }
}

BObjectImp* ObjArray::unpack( std::istream& is )
{
  unsigned arrsize;
  char colon;
  if ( !( is >> arrsize >> colon ) )
  {
    return new BError( "Unable to unpack array elemcount" );
  }
  if ( (int)arrsize < 0 )
  {
    return new BError( "Unable to unpack array elemcount. Invalid length!" );
  }
  if ( colon != ':' )
  {
    return new BError( "Unable to unpack array elemcount. Bad format. Colon not found!" );
  }
  std::unique_ptr<ObjArray> arr( new ObjArray );
  arr->ref_arr.resize( arrsize );
  for ( unsigned i = 0; i < arrsize; ++i )
  {
    BObjectImp* imp = BObjectImp::unpack( is );
    if ( imp != nullptr && !imp->isa( OTUninit ) )
    {
      arr->ref_arr[i].set( new BObject( imp ) );
    }
  }
  return arr.release();
}
}  // namespace Pol::Bscript
