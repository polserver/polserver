// Really for IDE integration
#include "bobject.h"

#include <ranges>

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
  auto* func = funcref->impptr_if<BFunctionRef>();
  if ( !func )
    return new BError( "Invalid parameter type" );

  // Add function arguments to value stack. Add arguments if there are not enough.  Remove if
  // there are too many
  while ( func->numParams() > static_cast<int>( args.size() ) )
  {
    args.push_back( BObjectRef( new BObject( UninitObject::create() ) ) );
  }

  // Resize args only for non-varadic functions
  if ( !func->variadic() )
    args.resize( func->numParams() );

  CallbackData<Callback>* details = new CallbackData<Callback>( callback );

  return new BContinuation(
      std::move( funcref ), std::move( args ),
      { CallbackData<Callback>::call, CallbackData<Callback>::free, CallbackData<Callback>::size },
      details );
}
}  // namespace Pol::Bscript

template <typename Callback>
inline void Pol::Bscript::Executor::walkCallStack( Callback callback )
{
  callback( PC );

  for ( auto& call : ControlStack | std::views::reverse )
  {
    callback( call.PC );
  }
}
