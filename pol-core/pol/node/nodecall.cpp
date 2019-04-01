/** @file
 *
 * @par History
 */

#include "../../bscript/impstr.h"
#include "../../clib/logfacility.h"
#include "../../clib/stlutil.h"
#include "../polclock.h"
#include "../uoexec.h"
#include "jsprog.h"
#include "module/objwrap.h"
#include "napi-wrap.h"
#include "nodecall.h"
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

    Napi::Array argv = Array::New( env, ex->ValueStack.size() );
    for ( size_t i = 0; !ex->ValueStack.empty(); )
    {
      Bscript::BObjectRef rightref = ex->ValueStack.back();
      ex->ValueStack.pop_back();

      Napi::Value convertedVal = Node::NodeObjectWrap::Wrap( env, rightref, request->reqId() );

      argv[i] = convertedVal;

      NODELOG.Format( "[{:04x}] [exec] argv[{}] = {}\n" )
          << request->reqId() << i << Node::ToUtf8Value( convertedVal );
    }
    try
    {
      auto ret =
          requireRef.Get( "scriptloader" )
              .As<Object>()
              .Get( "runScript" )
              .As<Function>()
              .Call( {env.Undefined() /* extUoExec */, Napi::String::New( env, prog->scriptname() ),
                      prog->obj.Value(), argv} );
      NODELOG.Format( "[{:04x}] [exec] returned {}\n" )
          << request->reqId() << Node::ToUtf8Value( ret );
    }
    catch ( std::exception& ex )
    {
      POLLOG_ERROR.Format( "Error running node script {}: {}\n" ) << prog->scriptname() << ex.what();
    }


    return clock();
  } );


  int theValue = call.getRef();
  NODELOG << "The clock was " << theValue << "\n";
}


}  // namespace Node
}  // namespace Pol
