/*
 * auxclient.h
 *
 *  Created on: Nov 22, 2011
 *      Author: kevin
 */

#ifndef AUXCLIENT_H_
#define AUXCLIENT_H_


#ifndef BSCRIPT_BOBJECT_H
#include "../../bscript/bobject.h"
#endif

#include "../../clib/socketsvc.h"
#include "../../clib/threadhelp.h"
#include "../../clib/weakptr.h"

#include "../scrdef.h"
#include "../uoexec.h"

#include <string>
#include <vector>

namespace Pol
{
namespace Plib
{
class Package;
};
namespace Clib
{
class ConfigElem;
};

namespace Network
{

class AuxClientThread;

class AuxConnection : public Bscript::BObjectImp
{
public:
  AuxConnection( AuxClientThread* auxclientthread, std::string ip ) :
    Bscript::BObjectImp( Bscript::BObjectImp::OTUnknown ),
    _auxclientthread( auxclientthread ),
    _ip( ip )
  {}

  virtual Bscript::BObjectImp* copy( ) const POL_OVERRIDE;
  virtual bool isTrue() const POL_OVERRIDE;
  virtual std::string getStringRep() const POL_OVERRIDE;
  virtual size_t sizeEstimate() const POL_OVERRIDE;

  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member( const char* membername ) POL_OVERRIDE;

  void disconnect();

private:
  AuxClientThread* _auxclientthread;
  std::string _ip;
};

class AuxService
{
public:
  AuxService( const Plib::Package* pkg, Clib::ConfigElem& elem );
  void run();
  size_t estimateSize() const;

  const Core::ScriptDef& scriptdef() const
  {
    return _scriptdef;
  }
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
  virtual void run() POL_OVERRIDE;
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
}
}

#endif /* AUXCLIENT_H_ */
