/** @file
 *
 * @par History
 * - 2010/02/03 Turley:    MethodScript support for mobiles
 * - 2011/12/01 MuadDib:   Changed max_skills to unsigned short. 0xFF max in packets anyway.
 * Probably never need to support over 255 skills anyway.
 */


#ifndef UOCLIENT_H
#define UOCLIENT_H

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "../clib/network/socketsvc.h"
#include "crypt/cryptkey.h"
#include "polclock.h"

namespace Pol
{
namespace Clib
{
class ConfigElem;
}
namespace Network
{
class Client;
}
namespace Core
{
class ExportScript;

class UoClientGeneral
{
public:
  void check( std::string& var, const char* tag, const char* deflt );
  size_t estimateSize() const;

  void deinitialize();

  //~UoClientGeneral(); Note: no deconstructor due to undefined static deconstruction order,
  // let it leak if the usual way fails

  class Mapping
  {
  public:
    bool any;
    std::string name;
    unsigned id;
    size_t estimateSize() const;
  };

  Mapping strength;
  Mapping intelligence;
  Mapping dexterity;

  Mapping hits;
  Mapping stamina;
  Mapping mana;
  unsigned short maxskills;     // dave changed 3/15/03, support configurable max skillid
  ExportScript* method_script;  // TODO deprecated!
};

class UoClientProtocol
{
public:
  UoClientProtocol();
  size_t estimateSize() const;
  bool EnableFlowControlPackets;
};

class UoClientListener;

class UoClientThread final : public Clib::SocketClientThread
{
public:
  UoClientThread( UoClientListener* def, Clib::Socket&& newsck );
  virtual void run() override;
  bool create();
  virtual ~UoClientThread() = default;

private:
  UoClientListener* _def;

public:
  Network::Client* client;
  poltime_t login_time;
};

class UoClientListener
{
public:
  UoClientListener( Clib::ConfigElem& elem );
  UoClientListener( const UoClientListener& ) = delete;
  UoClientListener& operator=( const UoClientListener& ) = delete;
  UoClientListener( UoClientListener&& ) = default;
  UoClientListener& operator=( UoClientListener&& ) = default;
  ~UoClientListener() = default;

  size_t estimateSize() const;

  Crypt::TCryptInfo encryption;
  unsigned short port;
  bool aosresist;
  bool sticky;
  std::list<std::unique_ptr<UoClientThread>> login_clients;
};
}  // namespace Core
}  // namespace Pol
#endif
