#pragma once

#include "bobject.h"
#include "exectype.h"

namespace Pol
{
namespace Bscript
{
class Executor;
class BContinuation;
class BFunctionRef;

struct ContinuationCallbackWrapper
{
  BObjectImp* ( *call )( Executor&, BContinuation*, void* /*wrapper data*/, BObjectRef /*result*/ );
  void ( *free )( void* );
  size_t ( *size )();
};

class BContinuation : public BObjectImp
{
public:
  BContinuation( BObjectRef funcref, ContinuationCallbackWrapper wrapper, void* wrapperData );
  ~BContinuation();

  BObjectImp* continueWith( Executor& exec, BObjectRef result );
  BFunctionRef* func() { return static_cast<BFunctionRef*>( funcref_->impptr() ); }
  size_t numParams() const { return static_cast<BFunctionRef*>( funcref_->impptr() )->numParams(); }

  BObjectImp* copy( void ) const override;
  size_t sizeEstimate() const override;
  std::string getStringRep() const override;

private:
  BObjectRef funcref_;
  ContinuationCallbackWrapper wrapper_;
  void* wrapperData_;
};
}  // namespace Bscript
}  // namespace Pol
