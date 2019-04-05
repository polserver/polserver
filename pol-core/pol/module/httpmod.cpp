/** @file
 *
 * @par History
 */

#include "httpmod.h"

#include "../../bscript/berror.h"
#include "../../bscript/impstr.h"
#include "../../clib/logfacility.h"
#include "../../clib/wnsckt.h"
#include "../../plib/systemstate.h"
#include "../uoexec.h"


namespace Pol
{
namespace Core
{
std::string http_decodestr( const std::string& s );
}
namespace Bscript
{
using namespace Module;
template <>
TmplExecutorModule<HttpExecutorModule>::FunctionTable
    TmplExecutorModule<HttpExecutorModule>::function_table = {
        {"WriteHtml", &HttpExecutorModule::mf_WriteHtml, UINT_MAX},
        {"WriteHtmlRaw", &HttpExecutorModule::mf_WriteHtmlRaw, UINT_MAX},
        {"QueryParam", &HttpExecutorModule::mf_QueryParam, UINT_MAX},
        {"QueryIP", &HttpExecutorModule::mf_QueryIP, UINT_MAX}
};
template <>
const char* TmplExecutorModule<HttpExecutorModule>::modname = "http";
}  // namespace Bscript
namespace Module
{
using namespace Bscript;

HttpExecutorModule::HttpExecutorModule( Bscript::Executor& exec, Clib::Socket& isck )
    : Bscript::TmplExecutorModule<HttpExecutorModule>( exec ),
      sck_( isck ),
      continuing_offset( 0 ),
      uoexec( static_cast<Core::UOExecutor&>( exec ) )
{
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
    const std::string& s = str->value();
    bool res =
        sck_.send_nowait( (void*)( s.c_str() + continuing_offset ),
                          static_cast<unsigned int>( s.length() - continuing_offset ), &nsent );
    if ( res )
    {
      continuing_offset = 0;
      // we don't really care if this works or not, terribly.
      sck_.send_nowait( "\n", 1, &nsent );
      return new BLong( 1 );
    }
    else
    {
      continuing_offset += nsent;
      uoexec.SleepForMs( 500 );
      --uoexec.PC;
      return uoexec.fparams[0]->impptr();
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
    const std::string& s = str->value();
    bool res =
        sck_.send_nowait( (void*)( s.c_str() + continuing_offset ),
                          static_cast<unsigned int>( s.length() - continuing_offset ), &nsent );
    if ( res )
    {
      continuing_offset = 0;
      return new BLong( 1 );
    }
    else
    {
      continuing_offset += nsent;
      uoexec.SleepForMs( 500 );
      --uoexec.PC;
      return uoexec.fparams[0]->impptr();
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
        INFO_PRINT << "http-param: '" << param << "', '" << Core::http_decodestr( param ) << "'\n";

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
  size_t size = sizeof( *this );
  for ( const auto& v : params_ )
    size += v.first.capacity() + v.second.capacity() + ( sizeof( void* ) * 3 + 1 ) / 2;
  return size;
}
}  // namespace Module
}  // namespace Pol
