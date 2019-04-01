/** @file
 *
 * @par History
 */

#include "../../clib/logfacility.h"
#include "../bscript/eprog.h"
#include "jsprog.h"
#include "nodecall.h"
#include "nodethread.h"

using namespace Napi;

namespace Pol
{
namespace Node
{
JavascriptProgram::JavascriptProgram() : Program() {}

Bscript::Program* JavascriptProgram::create()
{
  return new JavascriptProgram;
}

bool JavascriptProgram::hasProgram() const
{
  return false;
}

int JavascriptProgram::read( const char* fname )
{
  NODELOG << "[core] requesting " << fname << "\n";
  try
  {
    auto reqReturn = Node::makeCall<FunctionReference>(
        [fname]( Napi::Env env, NodeRequest<FunctionReference>* request ) {

          auto scriptName = std::string( "./" ) + fname;
          NODELOG.Format( "[{:04x}] [require] requesting, {}\n" ) << request->reqId() << scriptName;

          try
          {
            auto compiledWrapper = requireRef.Get( "scriptloader" )
                                       .As<Object>()
                                       .Get( "loadScript" )
                                       .As<Function>()
                                       .Call( {Napi::String::New( env, scriptName )} );

            compiledWrapper.As<Object>().Set(
                "_refId", String::New( env, std::string( "require(" ) + scriptName + ")@" +
                                                std::to_string( request->reqId() ) ) );

            NODELOG.Format( "[{:04x}] [require] compiled, {}\n" ) << request->reqId() << scriptName;


            return Napi::Persistent( compiledWrapper.As<Function>() );
          }
          catch ( std::exception& ex )
          {
            NODELOG.Format( "[{:04x}] [require] errored, {}\n" ) << request->reqId() << ex.what();
            return Napi::FunctionReference();
          }
        } );

    obj = reqReturn.getRef();

    if ( obj.IsEmpty() )
    {
      ERROR_PRINT << "Error loading javascript " << fname << ": No ObjectReference returned\n";
      return 1;
    }

    NODELOG << "[core] successful read for " << fname << "\n";
    name = fname;
    return 0;
  }
  catch ( std::exception& ex )
  {
    ERROR_PRINT << "Exception caught while loading node script " << fname << ": " << ex.what()
                << "\n";
  }
  return -1;
}

void JavascriptProgram::package( const Plib::Package* package )
{
  pkg = package;
}

std::string JavascriptProgram::scriptname() const
{
  return name;
}

Bscript::Program::ProgramType JavascriptProgram::type() const
{
  return Bscript::Program::JAVASCRIPT;
}

JavascriptProgram::~JavascriptProgram()
{
  // Node::release( std::move( obj ) );

  if ( Node::running && !obj.IsEmpty() )
  {
    auto call = Node::makeCall<bool>( [this]( Napi::Env env, NodeRequest<bool>* request ) {
      NODELOG.Format( "[{:04x}] [release] releasing program reference {}\n" )
          << request->reqId()
          << Node::ToUtf8Value( this->obj.Value().As<Object>().Get( "_refId" ) );

      this->obj.Unref();
      return true;
    } );
    call.getRef();
  }
}

}  // namespace Node
}  // namespace Pol
