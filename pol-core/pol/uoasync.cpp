
#include <string>
#include <time.h>

#include "../clib/logfacility.h"
#include "./network/cgdata.h"
#include "./network/client.h"
#include "./uoasync.h"
#include "./uoexec.h"

namespace Pol
{
namespace Module
{
class OSExecutorModule;
}
namespace Core
{
/** Request number */
std::atomic<unsigned int> UOAsyncRequest::nextRequestId = 0;

UOAsyncRequest::UOAsyncRequest( UOExecutor& exec, Mobile::Character* chr, Type type )
    : ref_counted(),
      exec_( exec ),
      chr_( chr ),
      handled_( false ),
      reqId_( UOAsyncRequest::nextRequestId++ ),
      type_( type ){};

UOAsyncRequest::~UOAsyncRequest()
{
  if ( !handled_ )
  {
    POLLOG_ERROR.Format(
        "Script Error! Request destroyed and unhandled! id = {}, chr = {}, script = {}, PC = "
        "{}\n" )
        << reqId_ << ( chr_ == nullptr ? "<no chr>" : chr_->name() ) << exec_.scriptname()
        << exec_.PC;
  }
}


void UOAsyncRequest::resolved( Bscript::BObjectImp* resp )
{
  if ( handled_ )
    return;
  handled_ = true;
  exec_.handleRequest( this, resp );
  if ( chr_ )
    chr_->client->gd->requests.removeRequest( this );
}

bool UOAsyncRequest::abort()
{
  resolved( nullptr );
  return true;
}

UOAsyncRequestHolder::UOAsyncRequestHolder() : requests(){};

void UOAsyncRequestHolder::addRequest( Core::UOAsyncRequest::Type type,
                                       ref_ptr<Core::UOAsyncRequest> req )
{
  requests[type].emplace_back( req );
}

bool UOAsyncRequestHolder::removeRequest( Core::UOAsyncRequest* req )
{
  auto reqs = requests.find( req->type_ );
  if ( reqs != requests.end() )
  {
    auto reqIter = std::find( reqs->second.begin(), reqs->second.end(), req );
    if ( reqIter != reqs->second.end() )
    {
      reqs->second.erase( reqIter );
      return true;
    }
  }
  return false;
}

int UOAsyncRequestHolder::abortAll()
{
  int aborted = 0;
  for ( auto a : requests )
  {
    for ( auto b : a.second )
    {
      if ( b->abort() )
        ++aborted;
    }
  }
  return aborted;
}


bool UOAsyncRequestHolder::hasRequest( Core::UOAsyncRequest::Type type )
{
  auto iter = requests.find( type );

  if ( iter != requests.end() )
  {
    // Since no hint provided, always return the first..
    auto req = iter->second;
    if ( !req.empty() )
    {
      return true;
    }
  }
  return false;
}
bool UOAsyncRequestHolder::hasRequest( Core::UOAsyncRequest::Type type, u32 hint )
{
  auto iter = requests.find( type );

  if ( iter != requests.end() )
  {
    for ( auto req : iter->second )
    {
      if ( req->reqId_ == hint )
        return true;
    }
  }
  return false;
}

}  // namespace Core
}  // namespace Pol
