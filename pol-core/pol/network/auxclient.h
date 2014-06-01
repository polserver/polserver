/*
 * auxclient.h
 *
 *  Created on: Nov 22, 2011
 *      Author: kevin
 */

#ifndef AUXCLIENT_H_
#define AUXCLIENT_H_

#include "../../bscript/bobject.h"
#include "../../clib/threadhelp.h"

namespace Pol {
  namespace Network {

	class AuxClientThread;

	class AuxConnection : public Bscript::BObjectImp
	{
	public:
	  AuxConnection( AuxClientThread* auxclientthread, string ip ) :
		Bscript::BObjectImp( Bscript::BObjectImp::OTUnknown ),
		_auxclientthread( auxclientthread ),
		_ip( ip )
	  {}

	  virtual Bscript::BObjectImp* copy( ) const;
	  virtual bool isTrue() const;
	  virtual std::string getStringRep() const;
	  virtual size_t sizeEstimate() const;

	  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
	  virtual Bscript::BObjectRef get_member( const char *membername );

	  void disconnect();

	private:
	  AuxClientThread* _auxclientthread;
	  string _ip;
	};

	class AuxService
	{
	public:
	  AuxService( const Plib::Package* pkg, Clib::ConfigElem& elem );
	  void run();

	  const Core::ScriptDef& scriptdef() const { return _scriptdef; }
	  std::vector<unsigned int> _aux_ip_match;
	  std::vector<unsigned int> _aux_ip_match_mask;
	private:
	  const Plib::Package* _pkg;
	  Core::ScriptDef _scriptdef;
	  unsigned short _port;
	};

	class AuxClientThread : public Clib::SocketClientThread
	{
	public:
	  AuxClientThread( AuxService* auxsvc, Clib::SocketListener& listener );
	  AuxClientThread( Core::ScriptDef scriptdef, Clib::Socket& sock );
	  virtual void run();
	  void transmit( const Bscript::BObjectImp* imp );
	  Bscript::BObjectImp* get_ip( );

	private:
	  bool init();
	  bool ipAllowed( sockaddr MyPeer );

	  AuxService* _auxservice;
	  Core::ScriptDef _scriptdef;
	  ref_ptr<AuxConnection> _auxconnection;
	  weak_ptr<Core::UOExecutor> _uoexec;
	};
#ifdef PERGOn
    extern std::unique_ptr<threadhelp::DynTaskThreadPool> auxthreadpool;
#endif
  }
}

#endif /* AUXCLIENT_H_ */
