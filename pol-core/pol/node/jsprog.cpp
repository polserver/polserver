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

std::future<void> Node::makeCall( std::function<Napi::Value( Napi::Env, Request* )>  ) {

  auto p = std::promise<void>();
  p.set_value();
  return p.get_future();
}

int JavascriptProgram::read( const char* fname )
{
  try
  {
    // auto fut = Node::require( std::string( "./" ) + fname );

    auto fut = Node::makeCall( [fname]( Napi::Env env, Request* request ) {
      auto req =
          requireRef.Value().As<Function>().Call( {Napi::String::New( env, fname )} ).As<Object>();

      auto funct = req.Get( "default" );

      auto functCode = funct.As<Object>()
                           .Get( "toString" )
                           .As<Function>()
                           .Call( funct, {} )
                           .As<String>()
                           .Utf8Value();


      req.Set( "_refId", String::New( env, std::string( "require(" ) + fname + ")@" +
                                               std::to_string( 1234 ) ) );

      NODELOG.Format( "[{:04x}] [require] resolved, {}\n" ) << request->reqId() << functCode;
      return env.Undefined();
      // return Napi::ObjectReference::New( req, 1 );
    } );

    fut.wait();  // waits until
                 // obj = fut.get();
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
  Node::release( std::move( obj ) );
}

}  // namespace Node
}  // namespace Pol
