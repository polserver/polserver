/*
History
=======

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include "../../bscript/impstr.h"
#include "../../bscript/berror.h"

#include "../../clib/wnsckt.h"

#include "../polcfg.h"
#include "../scrsched.h"
#include "../uoexec.h"

#include "httpmod.h"

template<>
TmplExecutorModule<HttpExecutorModule>::FunctionDef   
TmplExecutorModule<HttpExecutorModule>::function_table[] = 
{
	{ "WriteHtml",	  &HttpExecutorModule::mf_WriteHtml },
	{ "WriteHtmlRaw",   &HttpExecutorModule::mf_WriteHtmlRaw },	
	{ "QueryParam",	 &HttpExecutorModule::mf_QueryParam },
	{ "QueryIP",		&HttpExecutorModule::mf_QueryIP },
};

template<>
int TmplExecutorModule<HttpExecutorModule>::function_table_size = arsize(function_table);

BObjectImp* HttpExecutorModule::mf_WriteHtml()
{
	const String* str;
	if (!sck_.connected())
	{
		exec.seterror( true );
		return new BError( "Socket is disconnected" );
	}
	if (getStringParam( 0, str ))
	{
		// TODO: some tricky stuff so if the socket blocks, the script goes to
		// sleep for a bit and sends the rest later

		unsigned nsent;
		const string& s = str->value();
		bool res = sck_.send_nowait( (void *) (s.c_str()+continuing_offset), 
			s.length()-continuing_offset,
			&nsent);
		if (res)
		{
			continuing_offset = 0;
			// we don't really care if this works or not, terribly.
			sck_.send_nowait( "\n", 1, &nsent );
			return new BLong(1);
		}
		else
		{
			continuing_offset += nsent;
			uoexec.os_module->SleepForMs( 500 );
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
	if (!sck_.connected())
	{
		exec.seterror( true );
		return new BError( "Socket is disconnected" );
	}
	exec.makeString( 0 );
	if (getStringParam( 0, str ))
	{
		// TODO: some tricky stuff so if the socket blocks, the script goes to
		// sleep for a bit and sends the rest later

		unsigned nsent;
		const string& s = str->value();
		bool res = sck_.send_nowait( (void *) (s.c_str()+continuing_offset), 
			s.length()-continuing_offset,
			&nsent);
		if (res)
		{
			continuing_offset = 0;
			return new BLong(1);
		}
		else
		{
			continuing_offset += nsent;
			uoexec.os_module->SleepForMs( 500 );
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
	if (getStringParam( 0, str ))
	{
		QueryParamMap::iterator itr = params_.find( str->data() );
		if (itr != params_.end())
			return new String( (*itr).second );
		else
			return new BLong(0);
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

// set here to call from polwww.cpp correctly.
string http_decodestr( const string& s );

// query_string is everything after the '?'
void HttpExecutorModule::read_query_string( const string& query_string )
{
	if (!query_string.empty())
	{
		string::size_type brk;
		string::size_type start = 0;
		do {
			brk = query_string.find( '&', start );
			string param = query_string.substr( start, (brk == string::npos) ? brk : brk-start );

			string name, value;
			string::size_type eq = param.find( '=' );
			if (eq != string::npos)
			{
				name = param.substr( 0, eq );
				value = http_decodestr(param.substr( eq+1 ));
			}
			else
			{
				name = param;
				value = "";
			}
			params_[ name.c_str() ] = value;

			if (config.web_server_debug)
				cout << "http-param: '" << param << "', '" << http_decodestr( param ) << "'" << endl;

			start = brk+1;
		} while (brk != string::npos );
	}
}

void HttpExecutorModule::read_query_ip()
{
	query_ip_ = sck_.getpeername();
}