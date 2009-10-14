/*
History
=======

Notes
=======

*/

#ifndef __HTTPMOD_H
#define __HTTPMOD_H

class HttpExecutorModule : public TmplExecutorModule<HttpExecutorModule>
{
public:
	HttpExecutorModule( Executor& exec, Socket& isck ) :
	  TmplExecutorModule<HttpExecutorModule>( "http", exec ), 
		  sck_(isck),
		  continuing_offset(0),
		  uoexec( static_cast<UOExecutor&>(exec) ) {};

	  BObjectImp* mf_WriteHtml();
	  BObjectImp* mf_WriteHtmlRaw();
	  BObjectImp* mf_QueryParam();
	  BObjectImp* mf_QueryIP();

	  void read_query_string( const string& query_string );
	  void read_query_ip();

	  // TODO: clean up the socket ownership thing so these can be private again
public:
	Socket sck_;
	typedef std::map<string,string, ci_cmp_pred> QueryParamMap;
	QueryParamMap params_;
	int continuing_offset;
	UOExecutor& uoexec;
	std::string query_ip_;

};

#endif // HTTPMOD_H