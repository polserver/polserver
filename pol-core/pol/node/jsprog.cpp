/** @file
 *
 * @par History
 */

#include "jsprog.h"
#include "../../clib/logfacility.h"
#include "../bscript/eprog.h"
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
  try
  {
    auto reqReturn = Node::makeCall<ObjectReference>(
        [fname]( Napi::Env env, NodeRequest<ObjectReference>* request ) {

          auto scriptName = std::string( "./" ) + fname;
          NODELOG.Format( "[{:04x}] [require] requesting, {}\n" ) << request->reqId() << scriptName;

          try
          {
            auto requireVal = requireRef.Get( "require" )
                                  .As<Function>()
                                  .Call( {Napi::String::New( env, scriptName )} );
            auto requireObj = requireVal.As<Object>();

            auto funct = requireObj.Get( "default" );

            auto functCode = Node::ToUtf8Value( funct );

            requireObj.Set( "_refId",
                            String::New( env, std::string( "require(" ) + scriptName + ")@" +
                                                  std::to_string( request->reqId() ) ) );
            return ObjectReference::New( requireObj, 1 );

            NODELOG.Format( "[{:04x}] [require] resolved, {} = {}\n" )
                << request->reqId() << scriptName << functCode;
          }
          catch ( std::exception& ex )
          {
            NODELOG.Format( "[{:04x}] [require] errored, {}\n" ) << request->reqId() << scriptName;
            return Napi::Reference<Object>();
          }
        } );

    obj = reqReturn.getRef();

    if ( obj.IsEmpty() )
    {
      POLLOG_INFO << "Error reading javascript " << fname << "\n";
      return 1;
    }

    POLLOG_INFO << "Got a successful read for " << fname << "\n";
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


  auto call = Node::makeCall<bool>( [this]( Napi::Env env, NodeRequest<bool>* request ) {
    this->obj.Unref();
    return true;
  } );
  call.getRef();
}

}  // namespace Node
}  // namespace Pol
