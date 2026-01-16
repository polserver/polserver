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
  BContinuation( BObjectRef funcref, BObjectRefVec args, ContinuationCallbackWrapper wrapper,
                 void* wrapperData );
  ~BContinuation() override;

  BObjectImp* continueWith( Executor& exec, BObjectRef result );
  BFunctionRef* func();

  BObjectImp* copy() const override;
  size_t sizeEstimate() const override;
  std::string getStringRep() const override;

private:
  BObjectRef funcref_;
  ContinuationCallbackWrapper wrapper_;
  void* wrapperData_;

public:
  BObjectRefVec args;
};
}  // namespace Bscript
}  // namespace Pol
