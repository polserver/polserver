/** @file
 *
 * @par History
 */

#include "nodecall.h"
#include "../../bscript/impstr.h"
#include "../../clib/logfacility.h"
#include "../../clib/spinlock.h"
#include "../../clib/stlutil.h"
#include "../polclock.h"
#include "../uoexec.h"
#include "jsprog.h"
#include "module/objwrap.h"
#include "napi-wrap.h"
#include "nodethread.h"
#include <future>

#include "../uoscrobj.h"


using namespace Napi;

namespace Pol
{
namespace Node
{
unsigned long requestNumber = 0;

std::atomic_uint nextRequestId( 0 );


std::map<Core::UOExecutor*, Napi::ObjectReference> execToModuleMap;

// Only call this inside a Node environment!
// We are guaranteed the ObjectReference will not have been removed by the OnScriptReturn
//
Napi::Object GetRunningScript( Core::UOExecutor* uoexec )
{
  auto iter = execToModuleMap.find( uoexec );
  if ( iter != execToModuleMap.end() )
  {
    return iter->second.Value();
  }
  return Napi::Object();
}

bool emitEvent( Core::UOExecutor* exec, Core::EVENTID eventId, Core::ULWObject* target )
{
  return emitEvent( exec, new Pol::Module::UnsourcedEvent( eventId ), target );
}

// FIXME finish
// Call a function (non-blocking) on a UOExecutor's module
Napi::Value callFunc( Core::UOExecutor* exec, const std::string& funcName,
                      const std::initializer_list<Bscript::BObjectRef>& funcArgs,
                      unsigned long reqId = 0 )
{
  auto call = Node::makeCall<bool>(
      [=]( Napi::Env env, NodeRequest<bool>* request ) {
        Napi::Value retVal;

        Napi::Array argv = Array::New( env, funcArgs.size() );
        size_t i = 0;

        NODELOG.Format( "[{:04x}] [callFunc] calling {} on {}:{}\n" )
            << reqId << funcName << exec->pid() << exec->scriptname();
        auto iter = execToModuleMap.find( exec );

        if ( iter == execToModuleMap.end() )
        {
          NODELOG.Format( "[{:04x}] [callFunc] no module script found ({} {})\n" )
              << request->reqId() << iter->first->pid() << iter->first->scriptname();
          return env.Undefined();
        }

        else if ( iter->second.Value().Has( funcName ) )
        {
          NODELOG.Format( "[{:04x}] [callFunc] script has no function {}\n" )
              << request->reqId() << iter->first->pid() << iter->first->scriptname();
          return env.Undefined();
        }


        try
        {
          for ( auto arg = funcArgs.begin(); arg != funcArgs.end(); ++arg, ++i )
          {
            Bscript::BObjectRef argref = *arg;

            // FIXME THIS NEEDS TO BE SCRIPTMODULE
            Napi::Value convertedVal = Node::NodeObjectWrap::Wrap( env, argref, Object() );

            argv[i] = convertedVal;

            NODELOG.Format( "[{:04x}] [callFunc] argv[{}] = {}\n" )
                << reqId << i << Node::ToUtf8Value( convertedVal );
          }

          retVal = iter->second.Get( funcName )
                       .As<Object>()
                       .Get( "apply" )
                       .As<Function>()
                       .Call( iter->second.Value(), {argv} );

          NODELOG.Format( "[{:04x}] [callFunc] returned {}\n" )
              << request->reqId() << Node::ToUtf8Value( retVal );
        }
        catch ( std::exception& exc )
        {
          NODELOG.Format( "[{:04x}] [callFunc]  returned exception {}\n" )
              << request->reqId() << exc.what();
        }

        return retVal;
      },
      nullptr, false );

  return Value();
}
//
//// If exec is nullptr, it is a broadcast
// bool emitEvent( ObjectReference target, Bscript::BObjectImp* ev )
//{
//  passert( ev != nullptr );
//
//  auto call = Node::makeCall<bool>(
//      [ev, &target]( Napi::Env env, NodeRequest<bool>* request ) {
//        bool retVal = false, handled = false;
//        std::string eventName;
//        Bscript::BObjectRef objref( ev );
//
//        if ( ev->isa( Bscript::BObjectImp::OTStruct ) )
//        {
//          Bscript::BObjectRef type = ev->get_member( "type" );
//          eventName = type->impptr()->getStringRep();
//        }
//        else
//        {
//          eventName = ev->getStringRep();
//        }
//
//        // FIXME should this also be a script module...? it is a core generated event,
//        auto data = Node::NodeObjectWrap::Wrap( env, objref, Object(), request->reqId() );
//
//        NODELOG.Format( "[{:04x}] [exec] sending event to target {} {} {} \n" )
//            << request->reqId() << Node::ToUtf8Value( target.Value() ) << eventName
//            << Node::ToUtf8Value( data );
//        try
//        {
//          retVal = target.Get( "emit" )
//                       .As<Function>()
//                       .Call( target.Value(), {Napi::String::New( env, eventName ), data} )
//                       .ToBoolean()
//                       .Value();
//          NODELOG.Format( "[{:04x}] [exec] emit returned {}\n" )
//              << request->reqId() << eventName << retVal;
//        }
//        catch ( std::exception& exc )
//        {
//          retVal = false;
//          NODELOG.Format( "[{:04x}] [exec] script.emit {} returned exception {}\n" )
//              << request->reqId() << eventName << exc.what();
//        }
//
//        return retVal;
//      },
//      nullptr, false );
//
//  return call.getRef();
//}  // namespace Node

// If exec is nullptr, it is a broadcast
bool emitEvent( Core::UOExecutor* exec, Bscript::BObjectImp* ev, Core::ULWObject* target )
{
  passert( ev != nullptr );

  auto call = Node::makeCall<bool>(
      [=]( Napi::Env env, NodeRequest<bool>* request ) {
        bool retVal = false, handled = false;
        std::string eventName;
        Bscript::BObjectRef objref( ev );

        if ( ev->isa( Bscript::BObjectImp::OTStruct ) )
        {
          Bscript::BObjectRef type = ev->get_member( "type" );
          eventName = type->impptr()->getStringRep();
        }
        else
        {
          eventName = ev->getStringRep();
        }

        auto iter = ( exec == nullptr ? execToModuleMap.begin() : execToModuleMap.find( exec ) );

        try
        {
          do
          {
            if ( iter == execToModuleMap.end() )
              break;

            // Send to executor
            auto& scriptModule = iter->second;

            auto data =
                Node::NodeObjectWrap::Wrap( env, objref, scriptModule.Value(), request->reqId() );

            NODELOG.Format( "[{:04x}] [exec] sending event to script module ({} {}) {} {} {} \n" )
                << request->reqId() << iter->first->pid() << iter->first->scriptname() << eventName
                << Node::ToUtf8Value( data ) << objref->impptr()->getStringRep();

            retVal = iter->second.Get( "emit" )
                         .As<Function>()
                         .Call( scriptModule.Value(), {Napi::String::New( env, eventName ), data} )
                         .ToBoolean()
                         .Value();
            NODELOG.Format( "[{:04x}] [exec] script.emit {} returned {}\n" )
                << request->reqId() << eventName << retVal;
            handled |= retVal;
            iter++;

          } while ( true );

          // Send to target
          if ( target != nullptr )
          {
            auto globalObjrefMap = Node::requireRef.Get( "scriptloader" )
                                       .As<Object>()
                                       .Get( "ScriptModule" )
                                       .As<Object>()
                                       .Get( "globalObjrefMap" );
            auto objs = globalObjrefMap.As<Object>().Get( "get" ).As<Function>().Call(
                globalObjrefMap, {Number::New( env, target->serial )} );
            if ( objs.IsArray() )
            {
              auto arr = objs.As<Array>();
              size_t length = arr.Length();
              for ( size_t i = 0; i < length; ++i )
              {
                auto obj = arr.Get( i ).As<Object>();
                auto scriptModuleVal = obj.Get( "_mainModule" );
                auto data = Node::NodeObjectWrap::Wrap( env, objref, scriptModuleVal.As<Object>(),
                                                        request->reqId() );

                retVal = obj.Get( "emit" )
                             .As<Function>()
                             .Call( obj, {Napi::String::New( env, eventName ), data} )
                             .ToBoolean()
                             .Value();
                NODELOG.Format( "[{:04x}] [exec] script[].emit {} returned {}\n" )
                    << request->reqId() << eventName << retVal;
                handled |= retVal;
              }
            }
            else if ( objs.IsObject() )
            {
              auto obj = objs.As<Object>();
              auto scriptModuleVal = obj.Get( "_mainModule" );
              auto data = Node::NodeObjectWrap::Wrap( env, objref, scriptModuleVal.As<Object>(),
                                                      request->reqId() );

              retVal = obj.Get( "emit" )
                           .As<Function>()
                           .Call( obj, {Napi::String::New( env, eventName ), data} )
                           .ToBoolean()
                           .Value();
              NODELOG.Format( "[{:04x}] [exec] script.emit {} returned {}\n" )
                  << request->reqId() << eventName << retVal;
              handled |= retVal;
            }
          }
        }
        catch ( std::exception& exc )
        {
          retVal = false;
          NODELOG.Format( "[{:04x}] [exec] script.emit {} returned exception {}\n" )
              << request->reqId() << eventName << exc.what();
        }

        if ( !handled )
        {
          NODELOG.Format( "[{:04x}] [exec] event {} handled: {}\n" )
              << request->reqId() << eventName << handled;
          retVal = false;
        }
        return handled;
      },
      nullptr, false );

  return call.getRef();
}

struct pid_compare
{
  bool operator()( const Core::UOExecutor& lhs, const Core::UOExecutor& rhs ) const
  {
    return lhs.pid() < rhs.pid();
  }
};

Napi::Value OnScriptReturn( const CallbackInfo& cbinfo )
{
  Napi::Env env = cbinfo.Env();
  auto* req = static_cast<NodeRequest<Bscript::BObjectRef>*>( cbinfo.Data() );
  Core::UOExecutor* ex = req->uoexec_;
  Napi::Value obj = cbinfo.Length() > 0 ? cbinfo[0] : env.Undefined();

  NODELOG.Format( "[{:04x}] [exec] script returned, {} {}: {}\n" )
      << req->reqId() << ex->pid() << ex->scriptname() << Node::ToUtf8Value( obj );

  auto iter = execToModuleMap.find( ex );
  if ( iter != execToModuleMap.end() )
  {
    iter->second.Get( "removeAllListeners" ).As<Function>().Call( iter->second.Value(), {} );
    iter->second.Get( "clearReferences" ).As<Function>().Call( iter->second.Value(), {} );
    execToModuleMap.erase( iter );
  }
  bool ret = Core::scriptScheduler.free_externalscript( ex );
  NODELOG.Format( "[{:04x}] [exec] freeing executor: {}\n" ) << req->reqId() << ret;
  return Boolean::New( env, true );
}

Napi::Value OnScriptCatch( const CallbackInfo& cbinfo )
{
  Napi::Env env = cbinfo.Env();
  auto* req = static_cast<NodeRequest<Bscript::BObjectRef>*>( cbinfo.Data() );
  Core::UOExecutor* ex = req->uoexec_;
  Napi::Value obj = cbinfo.Length() > 0 ? cbinfo[0] : env.Undefined();

  NODELOG.Format( "[{:04x}] [exec] script error, {} {}: {}\n" )
      << req->reqId()

      << ex->pid() << ex->scriptname() << Node::ToUtf8Value( obj );

  auto iter = execToModuleMap.find( ex );
  if ( iter != execToModuleMap.end() )
  {
    iter->second.Get( "clearReferences" ).As<Function>().Call( iter->second.Value(), {} );
    execToModuleMap.erase( iter );
  }

  bool ret = Core::scriptScheduler.free_externalscript( ex );
  NODELOG.Format( "[{:04x}] [exec] freeing executor: {}\n" ) << req->reqId() << ret;

  return env.Global()
      .Get( "Promise" )
      .As<Object>()
      .Get( "resolve" )
      .As<Function>()
      .Call( env.Global().Get( "Promise" ), {Napi::Boolean::New( env, false )} );

  ;
  // return Boolean::New( env, false );
}

Bscript::BObjectRef runExecutor( Core::UOExecutor* ex )
{
  passert( ex->programType() == Bscript::Program::ProgramType::JAVASCRIPT );
  // Tell the script scheduler we will manage this executor ourselves

  NODELOG.Format(
      "[core] [exec] Adding executor {} {} to external holdlist (running to completion: {})\n" )
      << ex->pid() << ex->scriptname() << ex->running_to_completion();

  /*
  Add the executor to the script scheduler's "External Scripts" holdlist.

  Regarding telling the scheduler "we are done" is a little more complex. The External<>
  Finalize callback runs once the V8 garbage collector has freed the reference to the object.
  However, this is not done immediately after the script finishes -- the object is just
  arked as "ready for deletion". This means having _just_ an External<> Finalize is not enough
  to tell us when a script is finished in the context of async scripts (ie. a script that
  returns a Promise).

  This means we have two options regarding these async scripts:
    1. Trust the programmer resolves the promise when the script should end, and the UOExecutor
       is no longer needed.
       This means we can "free the script" in the scheduler immediately
       on the script's completion. However.
       Any use of NodeModuleWrap functions in the script will fail if the programmer does return
       a promise but and uses a module in another tick, eg. script returns non-Promise but has
       a timeout that uses a module.
       Also, a script that is "still running" but does not return a Promise will not be known
       to the core that it is running and will be lost.

    2. We'll tell the script scheduler we're finished with it in the External<UOExecutor>'s
       Finalize callback, which is ran with the garbage collector has freed the reference to
       our object. We won't know that a script is done until GC is ran. This is undeterministic
       unless we explicitly run it via `global.gc()`

    3. A combination of the two...?

  We'll go with OPTION 1.

  */
  Core::scriptScheduler.add_externalscript( ex );

  Node::JavascriptProgram* prog = static_cast<Node::JavascriptProgram*>( ex->prog_.get() );

  auto call = Node::makeCall<Bscript::BObjectRef>(
      [&]( Napi::Env env, NodeRequest<Bscript::BObjectRef>* request ) {
        auto obj = prog->obj.Value();
        auto reqId = request->reqId();
        NODELOG.Format( "[{:04x}] [exec] pid {}, call {} , argc {}\n" )
            << request->reqId() << ex->pid() << obj.Get( "_refId" ).As<String>().Utf8Value()
            << ex->ValueStack.size();
        // NODELOG.Clear();


        try
        {
          auto extUoExec =
              External<Core::UOExecutor>::New( env, ex, [=]( Napi::Env, Core::UOExecutor* uoexec ) {
                passert( uoexec != nullptr );
                NODELOG.Format( "[{:04x}] [exec] finalized\n" ) << reqId;
              } );

          auto jsCall = requireRef.Get( "scriptloader" )
                            .As<Object>()
                            .Get( "runScript" )
                            .As<Function>()
                            .Call( {extUoExec, Napi::String::New( env, prog->scriptname() ),
                                    prog->obj.Value(), Number::New( env, ex->pid() )} )
                            .As<Object>();

          // auto jsRetVal = jsCall.Get( "value" );
          // auto jsRetObj = jsRetVal.As<Object>();
          auto mod = jsCall.Get( "module" ).As<Object>();

          Napi::Array argv = Array::New( env, ex->ValueStack.size() );
          for ( size_t i = 0; !ex->ValueStack.empty(); ++i )
          {
            Bscript::BObjectRef rightref = ex->ValueStack.back();
            ex->ValueStack.pop_back();

            Napi::Value convertedVal = Node::NodeObjectWrap::Wrap( env, rightref, mod, reqId );

            argv[i] = convertedVal;

            NODELOG.Format( "[{:04x}] [exec] argv[{}] = {}\n" )
                << reqId << i << Node::ToUtf8Value( convertedVal );
          }
          auto jsRetVal = mod.Get( "run" ).As<Function>().Call( mod, {argv} );

          NODELOG.Format( "[{:04x}] [exec] returned value {}\n" )
              << request->reqId() << Node::ToUtf8Value( jsRetVal );

          // Add the module to a map of UOExecutor -> module, so we can trigger events
          // We will unreference and remove from map in the Return and Catch handlers below.
          execToModuleMap.emplace( ex, ObjectReference::New( mod, 1 ) );

          auto scriptRet = Napi::Function::New( env, OnScriptReturn, "OnScriptReturn", request );
          auto scriptCatch = Napi::Function::New( env, OnScriptCatch, "OnScriptCatch", request );
          auto convertedVal = NodeObjectWrap::Wrap( env, jsRetVal, reqId );

          if ( jsRetVal.IsPromise() )
          {
            auto thenVal =
                jsRetVal.As<Object>().Get( "then" ).As<Function>().Call( jsRetVal, {scriptRet} );
            thenVal.As<Object>()
                .Get( "catch" )
                .As<Function>()
                .Call( thenVal, {scriptCatch} )
                .As<Object>();
          }
          else
          {
            scriptRet.Call( jsRetVal, {jsRetVal} );
          }

          return convertedVal;
        }
        catch ( std::exception& e )
        {
          NODELOG.Format( "[{:04x}] [exec] errored {}\n" ) << request->reqId() << e.what();
          POLLOG_ERROR.Format( "Error running node script {}: {}\n" )
              << prog->scriptname() << e.what();

          auto scriptCatch = Napi::Function::New( env, OnScriptCatch, "OnScriptCatch", request );
          scriptCatch.Call( {String::New( env, e.what() )} );

          auto convertedVal = Bscript::BObjectRef( new Bscript::BError( e.what() ) );
          return convertedVal;
        }
      },
      ex );

  auto impref = call.getRef();
  if ( impref.get() == nullptr )
  {
    NODELOG.Format( "[{:04x}] [exec] errored!\n" ) << call.reqId();
    return Bscript::BObjectRef( Bscript::UninitObject::SharedInstance );
  }
  NODELOG.Format( "[{:04x}] [exec] returned to core {}\n" )
      << call.reqId() << impref->impptr()->getStringRep();
  return impref;
}


}  // namespace Node
}  // namespace Pol
