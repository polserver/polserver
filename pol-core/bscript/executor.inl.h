// Really for IDE integration
#include "bobject.h"

namespace Pol::Bscript
{
template <typename Callback>
struct CallbackData
{
  CallbackData( Callback callback ) : callback( callback ) {}

  static BObjectImp* call( Executor& exec, BContinuation* continuation, void* data,
                           BObjectRef result )
  {
    CallbackData* cbData = static_cast<CallbackData*>( data );

    return cbData->callback( exec, continuation, result );
  }

  static void free( void* data )
  {
    CallbackData* cbData = static_cast<CallbackData*>( data );

    delete cbData;
  }

  static size_t size() { return sizeof( CallbackData ); }

private:
  Callback callback;
};

// returns BError* or BContinuation*
template <typename Callback>
BObjectImp* Executor::makeContinuation( BObjectRef funcref, Callback callback, BObjectRefVec args )
{
  if ( !funcref->isa( BObjectImp::OTFuncRef ) )
  {
    return new BError( "Invalid parameter type" );
  }

  auto func = static_cast<BFunctionRef*>( funcref->impptr() );

  // Add function reference to stack
  ValueStack.push_back( funcref );

  // Add function arguments to value stack. Add arguments if there are not enough.  Remove if
  // there are too many
  while ( func->numParams() > args.size() )
  {
    args.push_back( BObjectRef( new BObject( UninitObject::create() ) ) );
  }
  // Resize and erase the state in `args` since it was moved above.
  args.resize( func->numParams() );

  // Move all arguments to the value stack
  ValueStack.insert( ValueStack.end(), std::make_move_iterator( args.begin() ),
                     std::make_move_iterator( args.end() ) );
  args.erase( args.begin(), args.end() );

  CallbackData<Callback>* details = new CallbackData<Callback>( callback );

  return new BContinuation(
      std::move( funcref ),
      { CallbackData<Callback>::call, CallbackData<Callback>::free, CallbackData<Callback>::size },
      details );
}
}  // namespace Pol::Bscript