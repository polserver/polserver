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

#include <atomic>
#include <string>
#include <vector>

#include "../../clib/network/socketsvc.h"
#include "../../clib/refptr.h"
#include "../../clib/weakptr.h"
#include "../polobject.h"
#include "../scrdef.h"
#include "../uoexec.h"

namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
namespace Clib
{
class Socket;
}  // namespace Clib
namespace Core
{
class UOExecutor;
}  // namespace Core
}  // namespace Pol
struct sockaddr;

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

class AuxConnection final : public Core::PolObjectImp
{
public:
  AuxConnection( AuxClientThread* auxclientthread, std::string ip )
      : PolObjectImp( Bscript::BObjectImp::OTUnknown ),
        _auxclientthread( auxclientthread ),
        _ip( ip )
  {
  }

  virtual Bscript::BObjectImp* copy() const override;
  virtual bool isTrue() const override;
  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override;

  virtual Bscript::BObjectImp* call_polmethod( const char* methodname,
                                               Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;

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

  const Core::ScriptDef& scriptdef() const { return _scriptdef; }
  std::vector<unsigned int> _aux_ip_match;
  std::vector<unsigned int> _aux_ip_match_mask;

private:
  const Plib::Package* _pkg;
  Core::ScriptDef _scriptdef;
  unsigned short _port;
};

class AuxClientThread final : public Clib::SocketClientThread
{
public:
  AuxClientThread( AuxService* auxsvc, Clib::Socket&& sock );
  AuxClientThread( Core::ScriptDef scriptdef, Clib::Socket&& sock, Bscript::BObjectImp* params,
                   bool assume_string, bool keep_alive, bool use_byte_reader );
  virtual void run() override;
  void transmit( const Bscript::BObjectImp* imp );
  Bscript::BObjectImp* get_ip();

private:
  bool init();
  bool ipAllowed( sockaddr MyPeer );
  void transmit( const std::string& msg );
  AuxService* _auxservice;
  ref_ptr<AuxConnection> _auxconnection;
  weak_ptr<Core::UOExecutor> _uoexec;
  Core::ScriptDef _scriptdef;
  Bscript::BObjectImp* _params;
  bool _assume_string;
  std::atomic<int> _transmit_counter;
  bool _keep_alive;
  bool _use_byte_reader;
};
}  // namespace Network
}  // namespace Pol

#endif /* AUXCLIENT_H_ */
