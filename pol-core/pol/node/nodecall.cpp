/** @file
 *
 * @par History
 */

#include "nodecall.h"
#include "../../clib/logfacility.h"
#include "../../clib/stlutil.h"
#include "../polclock.h"
#include "../uoexec.h"
#include "jsprog.h"
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

    auto argv = std::vector<Napi::Value>();
    for ( size_t i = 0; !ex->ValueStack.empty(); )
    {
      Bscript::BObjectRef rightref = ex->ValueStack.back();
      ex->ValueStack.pop_back();

      // Lets do basic conversions here...
      Napi::Value convertedVal;
      auto* impptr = rightref.get()->impptr();
      // auto x = Napi::External<Bscript::BObjectImp>::New( env, impptr );

      // NODELOG << "To string: " << x.ToString().Utf8Value() << "\n";
      auto val = env.Global().Has( "Proxy" );
      if ( val )
      {
        NODELOG << "WE HAVE PROXY OBJECT!\n";
      }

      if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTLong ) )
      {
        NODELOG << "ITS A LONG!\n";

        auto longptr = impptr;
        // NodeObjectWrap longwrap = NodeObjectWrap::New( env, impptr );
        Bscript::BLong* aob = Clib::explicit_cast<Bscript::BLong*, Bscript::BObjectImp*>( impptr );
        convertedVal = Napi::Number::New( env, aob->value() );
      }
      else
      {
        NODELOG.Format( "[{:04x}] [exec] error converting arg {}\n" ) << request->reqId() << i;
        convertedVal = env.Undefined();
      }
      argv.push_back( convertedVal );
      auto last = argv.back();
      NODELOG.Format( "[{:04x}] [exec] argv[{}] = {}\n" )
          << request->reqId() << i << Node::ToUtf8Value( last );
    }
    // TODO pass args
    auto ret = obj.Get( "default" ).As<Function>().Call( {} );

    // auto retString = Node::ToUtf8Value( ret );
    NODELOG.Format( "[{:04x}] [exec] returned {}\n" ) << request->reqId() << "";

    return clock();
  } );


  int theValue = call.getRef();
  NODELOG << "The clock was " << theValue << "\n";
}


}  // namespace Node
}  // namespace Pol
