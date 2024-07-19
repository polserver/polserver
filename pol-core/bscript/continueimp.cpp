#include "continueimp.h"
#include "executor.h"

namespace Pol
{
namespace Bscript
{
BContinuationImp::BContinuationImp( BObjectRef funcref, ContinuationCallbackWrapper wrapper,
                                    void* wrapperData )
    : BObjectImp( BObjectImp::OTContinuation ),
      funcref_( std::move( funcref ) ),
      wrapper_( wrapper ),
      wrapperData_( wrapperData )
{
}

BContinuationImp::~BContinuationImp()
{
  wrapper_.free( wrapperData_ );
}

BObjectImp* BContinuationImp::continueWith( Executor& exec, BObjectRef result )
{
  return wrapper_.call( exec, this, wrapperData_, std::move( result ) );
}

BObjectImp* BContinuationImp::copy( void ) const
{
  return nullptr;
}

size_t BContinuationImp::sizeEstimate() const
{
  return sizeof( BContinuationImp ) + wrapper_.size();
}

std::string BContinuationImp::getStringRep() const
{
  return "<continuation>";
}
}  // namespace Bscript
}  // namespace Pol
