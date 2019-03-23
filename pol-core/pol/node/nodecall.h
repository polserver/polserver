/** @file
 *
 * @par History
 */


#ifndef NODECALL_H
#define NODECALL_H

#include "../../clib/timer.h"
#include "napi-wrap.h"
#include <atomic>
#include <future>
#include <tuple>

using namespace Napi;

namespace Pol
{
namespace Node
{
class Request
{
private:
  using timestamp = Tools::HighPerfTimer::time_mu;
  using timepoint = std::tuple<std::string, timestamp>;
  unsigned long reqId_;
  Tools::HighPerfTimer timer_;
  std::vector<timepoint> points_;
  Napi::Env env_;
  static std::atomic_uint nextRequestId;

public:
  Request( Napi::Env env );
  unsigned int reqId() const;
  timestamp checkpoint( const std::string& key );
};


std::future<Napi::ObjectReference> require( const std::string& name );

std::future<bool> release( Napi::ObjectReference ref );

std::future<bool> call( Napi::ObjectReference& ref );


std::future<void> makeCall( std::function<Napi::Value( Napi::Env, Request* )> );

}  // namespace Node
}  // namespace Pol

#endif
