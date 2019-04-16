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


std::map<Core::UOExecutor*, Napi::ObjectReference> execToModuleMap;

void emitExecutorShutdowns()
{
  auto call = Node::makeCall<bool>( [&]( Napi::Env env, NodeRequest<bool>* request ) {
    for ( auto& iter : execToModuleMap )
    {
      NODELOG.Format( "[{:04x}] [exec] finalizing executor {}\n" )
          << request->reqId() << iter.first->scriptname();
      bool retVal = iter.second.Get( "emit" )
                        .As<Function>()
                        .Call( iter.second.Value(), {Napi::String::New( env, "shutdown" )} )
                        .ToBoolean()
                        .Value();
      NODELOG.Format( "[{:04x}] [exec] module.emit('shutdown') returned {}\n" )
          << request->reqId() << retVal;
      iter.second.Unref();
    }
    return true;
  } );
  call.getRef();
  execToModuleMap.clear();
}

struct pid_compare
{
  bool operator()( const Core::UOExecutor& lhs, const Core::UOExecutor& rhs ) const
  {
    return lhs.pid() < rhs.pid();
  }
};

Bscript::BObjectImp* runExecutor( Core::UOExecutor* ex )
{
  passert( ex->programType() == Bscript::Program::ProgramType::JAVASCRIPT );
  // Tell the script scheduler we will manage this executor ourselves

  NODELOG.Format(
      "[core] [exec] Adding executor {} {} to external holdlist (running to completion: {})\n" )
      << ex->pid() << ex->scriptname() << ex->running_to_completion();

  Core::scriptScheduler.add_externalscript( ex );

  // execs.emplace( std::move( Core::UOExecutor() ) );

  Node::JavascriptProgram* prog = static_cast<Node::JavascriptProgram*>( ex->prog_.get() );

  auto call = Node::makeCall<Bscript::BObjectImp*>(
      [&]( Napi::Env env, NodeRequest<Bscript::BObjectImp*>* request ) {
        auto obj = prog->obj.Value();
        auto reqId = request->reqId();
        NODELOG.Format( "[{:04x}] [exec] pid {}, call {} , argc {}\n" )
            << request->reqId() << ex->pid() << obj.Get( "_refId" ).As<String>().Utf8Value()
            << ex->ValueStack.size();
        // NODELOG.Clear();

        Napi::Array argv = Array::New( env, ex->ValueStack.size() );
        for ( size_t i = 0; !ex->ValueStack.empty(); ++i )
        {
          Bscript::BObjectRef rightref = ex->ValueStack.back();
          ex->ValueStack.pop_back();

          Napi::Value convertedVal = Node::NodeObjectWrap::Wrap( env, rightref, reqId );

          argv[i] = convertedVal;

          NODELOG.Format( "[{:04x}] [exec] argv[{}] = {}\n" )
              << reqId << i << Node::ToUtf8Value( convertedVal );
        }

        try
        {
          auto jsCall =
              requireRef.Get( "scriptloader" )
                  .As<Object>()
                  .Get( "runScript" )
                  .As<Function>()
                  .Call( {External<Core::UOExecutor>::New(
                              env, ex,
                              [=]( Napi::Env, Core::UOExecutor* uoexec ) {
                                passert( uoexec != nullptr );

                                NODELOG.Format(
                                    "[{:04x}] [exec] pid {} "
                                    "finalized\n" )
                                    << reqId << uoexec->pid();
                                Core::scriptScheduler.free_externalscript( uoexec );
                              } ),
                          Napi::String::New( env, prog->scriptname() ), prog->obj.Value(), argv} )
                  .As<Object>();

          auto jsRetVal = jsCall.Get( "value" );
          NODELOG.Format( "[{:04x}] [exec] returned value {}\n" )
              << request->reqId() << Node::ToUtf8Value( jsRetVal );
          if ( !ex->running_to_completion() )
          {
            auto mod = jsCall.Get( "module" ).As<Object>();
            // auto emplaceRes = execs.emplace( std::make_pair(ex, ObjectReference::New(mod,1)) );
            // auto emplaceRes = execs.emplace( std::make_pair(ex,ObjectReference()) ); // ex,
            // ObjectReference() );

            if ( mod.Get( "_eventsCount" ).As<Number>().Int32Value() > 0 )
              execToModuleMap.emplace( ex, ObjectReference::New( mod, 1 ) );
            // emplaceRes.first->second.Reset .emplace( ex, ObjectReference::New( mod, 1 ) );
          }
          // execToModuleMap.find( ex );

          return NodeObjectWrap::Wrap( env, jsRetVal, reqId )->impptr()->copy();
        }
        catch ( std::exception& ex )
        {
          NODELOG.Format( "[{:04x}] [exec] errored {}\n" ) << request->reqId() << ex.what();
          POLLOG_ERROR.Format( "Error running node script {}: {}\n" )
              << prog->scriptname() << ex.what();
          return static_cast<Bscript::BObjectImp*>( new Bscript::BError(ex.what()) );
        }
      } );

  auto* impptr = call.getRef();
  NODELOG.Format( "[{:04x}] [exec] returned to core {}\n" )
      << call.reqId() << impptr->getStringRep();
  return impptr;

  // Bscript::UninitObject::create();
}


}  // namespace Node
}  // namespace Pol
