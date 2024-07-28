/** @file
 *
 * @par History
 */

#include "httpmod.h"
#include "../../bscript/berror.h"
#include "../../bscript/impstr.h"
#include "../../clib/logfacility.h"
#include "../../clib/network/wnsckt.h"
#include "../../clib/stlutil.h"
#include "../../plib/systemstate.h"
#include "../uoexec.h"

#include <module_defs/http.h>

namespace Pol
{
namespace Core
{
std::string http_decodestr( const std::string& s );
std::string reasonPhrase( int code );
}  // namespace Core

namespace Module
{
using namespace Bscript;

HttpExecutorModule::HttpExecutorModule( Bscript::Executor& exec, Clib::Socket&& isck )
    : Bscript::TmplExecutorModule<HttpExecutorModule, Core::PolModule>( exec ),
      sck_( std::move( isck ) ),
      continuing_offset( 0 )
{
}

HttpExecutorModule::~HttpExecutorModule()
{
  if ( sck_.connected() )
  {
    unsigned nsent;
    std::string line;

    // Since status line and headers are now controlled by script, check if they are sendable.
    if ( !cannotSendStatus )
    {
      line += "HTTP/1.1 200 OK\n";
    }
    if ( !cannotSendHeaders )
    {
      if ( !hasCustomContentType )
      {
        line += "Content-Type: text/html\n\n";
      }
      else
      {
        line += "\n";
      }

      if ( !line.empty() )
      {
        sck_.send_nowait( (void*)( line.c_str() ), static_cast<unsigned int>( line.length() ),
                          &nsent );
      }
    }
  }
}

BObjectImp* HttpExecutorModule::mf_WriteStatus()
{
  int code;
  const String* reason;

  if ( !sck_.connected() )
  {
    exec.seterror( true );
    return new BError( "Socket is disconnected" );
  }

  if ( getParam( 0, code ) && getStringParam( 1, reason ) )
  {
    if ( cannotSendStatus )
    {
      return new BError(
          "Cannot send status after WriteStatus, WriteHeader, WriteHtml, or WriteHtmlRaw" );
    }

    unsigned nsent;
    std::string line = "HTTP/1.1 " + std::to_string( code );
    auto& reasonString = reason->value();

    if ( reasonString.empty() )
    {
      std::string defaultReason = Core::reasonPhrase( code );
      if ( !defaultReason.empty() )
      {
        line += " " + defaultReason + "\n";
      }
    }
    else
    {
      line += " " + reasonString + "\n";
    }

    bool res =
        sck_.send_nowait( (void*)( line.c_str() + continuing_offset ),
                          static_cast<unsigned int>( line.length() - continuing_offset ), &nsent );

    if ( res )
    {
      cannotSendStatus = true;
      continuing_offset = 0;
      return new BLong( 1 );
    }
    else
    {
      continuing_offset += nsent;
      auto& uoex = uoexec();
      uoex.SleepForMs( 500u );
      --uoex.PC;
      return uoex.fparams[0]->impptr();
    }
  }
  return new BError( "Invalid parameter type" );
}


BObjectImp* HttpExecutorModule::mf_WriteHeader()
{
  const String* name;
  const String* value;

  if ( !sck_.connected() )
  {
    exec.seterror( true );
    return new BError( "Socket is disconnected" );
  }

  if ( getStringParam( 0, name ) && getStringParam( 1, value ) )
  {
    if ( cannotSendHeaders )
    {
      return new BError( "Cannot send headers after WriteHtml or WriteHtmlRaw" );
    }

    if ( Clib::strlowerASCII( name->value() ) == "content-type" )
    {
      hasCustomContentType = true;
    }

    unsigned nsent;
    std::string line;

    if ( !cannotSendStatus )
    {
      line += "HTTP/1.1 200 OK\n";
    }
    line += name->value() + ": " + value->value() + "\n";

    bool res =
        sck_.send_nowait( (void*)( line.c_str() + continuing_offset ),
                          static_cast<unsigned int>( line.length() - continuing_offset ), &nsent );

    if ( res )
    {
      cannotSendStatus = true;
      continuing_offset = 0;
      return new BLong( 1 );
    }
    else
    {
      continuing_offset += nsent;
      auto& uoex = uoexec();
      uoex.SleepForMs( 500u );
      --uoex.PC;
      return uoex.fparams[0]->impptr();
    }
  }
  return new BError( "Invalid parameter type" );
}

BObjectImp* HttpExecutorModule::mf_WriteHtml()
{
  const String* str;
  if ( !sck_.connected() )
  {
    exec.seterror( true );
    return new BError( "Socket is disconnected" );
  }
  if ( getStringParam( 0, str ) )
  {
    // TODO: some tricky stuff so if the socket blocks, the script goes to
    // sleep for a bit and sends the rest later
    unsigned nsent;

    std::string s;
    if ( !cannotSendStatus )
    {
      s += "HTTP/1.1 200 OK\n";
    }

    if ( !cannotSendHeaders )
    {
      if ( !hasCustomContentType )
      {
        s += "Content-Type: text/html\n";
      }
      s += "\n";
    }
    s += str->value();

    bool res =
        sck_.send_nowait( (void*)( s.c_str() + continuing_offset ),
                          static_cast<unsigned int>( s.length() - continuing_offset ), &nsent );
    if ( res )
    {
      cannotSendStatus = true;
      cannotSendHeaders = true;
      continuing_offset = 0;
      // we don't really care if this works or not, terribly.
      sck_.send_nowait( "\n", 1, &nsent );
      return new BLong( 1 );
    }
    else
    {
      continuing_offset += nsent;
      auto& uoex = uoexec();
      uoex.SleepForMs( 500u );
      --uoex.PC;
      return uoex.fparams[0]->impptr();
    }
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* HttpExecutorModule::mf_WriteHtmlRaw()
{
  const String* str;
  if ( !sck_.connected() )
  {
    exec.seterror( true );
    return new BError( "Socket is disconnected" );
  }
  exec.makeString( 0 );
  if ( getStringParam( 0, str ) )
  {
    // TODO: some tricky stuff so if the socket blocks, the script goes to
    // sleep for a bit and sends the rest later

    unsigned nsent;
    std::string s;

    if ( !cannotSendStatus )
    {
      s += "HTTP/1.1 200 OK\n";
    }

    if ( !cannotSendHeaders )
    {
      if ( !hasCustomContentType )
      {
        s += "Content-Type: text/html\n";
      }
      s += "\n";
    }
    s += str->value();

    bool res =
        sck_.send_nowait( (void*)( s.c_str() + continuing_offset ),
                          static_cast<unsigned int>( s.length() - continuing_offset ), &nsent );
    if ( res )
    {
      cannotSendStatus = true;
      cannotSendHeaders = true;
      continuing_offset = 0;
      return new BLong( 1 );
    }
    else
    {
      continuing_offset += nsent;
      auto& uoex = uoexec();
      uoex.SleepForMs( 500u );
      --uoex.PC;
      return uoex.fparams[0]->impptr();
    }
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

#if 0
  BObjectImp* HttpExecutorModule::mf_WriteHtml()
  {
    const String* str;
    if (getStringParam( 0, str ))
    {
    // TODO: some tricky stuff so if the socket blocks, the script goes to
    // sleep for a bit and sends the rest later
    http_writeline( sck_, str->value() );
    return new BLong(1);
    }
    else
    {
    return new BError( "Invalid parameter type" );
    }
  }
#endif

BObjectImp* HttpExecutorModule::mf_QueryParam()
{
  const String* str;
  if ( getStringParam( 0, str ) )
  {
    QueryParamMap::iterator itr = params_.find( str->data() );
    if ( itr != params_.end() )
      return new String( ( *itr ).second );
    else
      return new BLong( 0 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* HttpExecutorModule::mf_QueryIP()
{
  return new String( query_ip_ );
}


// query_string is everything after the '?'
void HttpExecutorModule::read_query_string( const std::string& query_string )
{
  if ( !query_string.empty() )
  {
    std::string::size_type brk;
    std::string::size_type start = 0;
    do
    {
      brk = query_string.find( '&', start );
      std::string param =
          query_string.substr( start, ( brk == std::string::npos ) ? brk : brk - start );

      std::string name, value;
      std::string::size_type eq = param.find( '=' );
      if ( eq != std::string::npos )
      {
        name = param.substr( 0, eq );
        value = Core::http_decodestr( param.substr( eq + 1 ) );
      }
      else
      {
        name = param;
        value = "";
      }
      params_[name.c_str()] = value;

      if ( Plib::systemstate.config.web_server_debug )
        INFO_PRINTLN( "http-param: '{}', '{}'", param, Core::http_decodestr( param ) );

      start = brk + 1;
    } while ( brk != std::string::npos );
  }
}

void HttpExecutorModule::read_query_ip()
{
  query_ip_ = sck_.getpeername();
}

size_t HttpExecutorModule::sizeEstimate() const
{
  size_t size = sizeof( *this ) + Clib::memsize( params_ );
  return size;
}
}  // namespace Module
}  // namespace Pol
