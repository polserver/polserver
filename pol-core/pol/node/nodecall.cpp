/** @file
 *
 * @par History
 */

#include "nodecall.h"
#include "../../bscript/impstr.h"
#include "../../clib/logfacility.h"
#include "../../clib/stlutil.h"
#include "../polclock.h"
#include "../uoexec.h"
#include "jsprog.h"
#include "module/objwrap.h"
#include "napi-wrap.h"
#include "nodethread.h"
#include <future>

using namespace Napi;

namespace Pol
{
namespace Node
{
unsigned long requestNumber = 0;

std::atomic_uint nextRequestId( 0 );

void callProgram( Node::JavascriptProgram* prog, Core::UOExecutor* ex )
{
  auto call = Node::makeCall<int>( [prog, ex]( Napi::Env env, NodeRequest<int>* request ) {

    auto obj = prog->obj.Value();
    NODELOG.Format( "[{:04x}] [exec] call {} , argc {}\n" )
        << request->reqId() << obj.Get( "_refId" ).As<String>().Utf8Value()
        << ex->ValueStack.size();

    // std::vector<Napi::Value> argv;
    std::vector<napi_value> argv;
    for ( size_t i = 0; !ex->ValueStack.empty(); )
    {
      Bscript::BObjectRef rightref = ex->ValueStack.back();
      ex->ValueStack.pop_back();

      Napi::Value convertedVal = Node::NodeObjectWrap::Wrap( env, rightref, request->reqId() );

      argv.push_back( convertedVal );


      NODELOG.Format( "[{:04x}] [exec] argv[{}] = {}\n" )
          << request->reqId() << i << Node::ToUtf8Value( convertedVal );
    }
    // TODO pass args
    auto funct = obj.Get( "default" );
    auto ret = funct.As<Function>().Call( argv );

    // auto retString = Node::ToUtf8Value( ret );
    NODELOG.Format( "[{:04x}] [exec] returned {}\n" )
        << request->reqId() << Node::ToUtf8Value( ret );

    return clock();
  } );


  int theValue = call.getRef();
  NODELOG << "The clock was " << theValue << "\n";
}


}  // namespace Node
}  // namespace Pol
