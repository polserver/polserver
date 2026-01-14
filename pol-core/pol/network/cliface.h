/** @file
 *
 * @par History
 */


#ifndef CLIFACE_H
#define CLIFACE_H

#include <vector>
namespace Pol
{
namespace Mobile
{
class Character;
class Attribute;
}  // namespace Mobile
namespace Core
{
class Vital;
class UOSkill;
}  // namespace Core

namespace Network
{
class Client;

struct ClientVitalUpdaters
{
public:
  ClientVitalUpdaters();

  void ( *my_vital_changed )( Client* client, Mobile::Character* me, const Core::Vital* vital );
  void ( *others_vital_changed )( Client* client, Mobile::Character* him,
                                  const Core::Vital* vital );
};

struct ClientAttributeUpdaters
{
public:
  ClientAttributeUpdaters();

  void ( *my_attr_changed )( Client* client, Mobile::Character* me, const Mobile::Attribute* attr );

  const Core::UOSkill* pUOSkill;
};

class ClientInterface
{
public:
  virtual ~ClientInterface() = default;
  void register_client( Client* client );
  void deregister_client( Client* client );

  static void tell_vital_changed( Mobile::Character* who, const Core::Vital* vital );
  static void tell_attribute_changed( Mobile::Character* who, const Mobile::Attribute* attr );

protected:
  virtual void Initialize();

  virtual void bcast_vital_changed( Mobile::Character* who, const Core::Vital* vital ) const = 0;

  std::vector<ClientVitalUpdaters> vital_updaters;
  std::vector<ClientAttributeUpdaters> attribute_updaters;

  std::vector<Client*> clients;

  friend void send_uo_skill( Client* client, Mobile::Character* me, const Mobile::Attribute* attr );
};

class UOClientInterface final : public ClientInterface
{
public:
  void Initialize() override;

protected:
  friend class ClientInterface;
  void bcast_vital_changed( Mobile::Character* who, const Core::Vital* vital ) const override;
};
}  // namespace Network
}  // namespace Pol
#endif
