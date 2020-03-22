#ifndef GUILDS_H
#define GUILDS_H

#include "../clib/refptr.h"
#include "network/msghandl.h"
#include "proplist.h"

namespace Pol
{
namespace Clib
{
class ConfigElem;
class StreamWriter;
}  // namespace Clib
namespace Mobile
{
class Character;
}
namespace Module
{
class EGuildRefObjImp;
}
namespace Core
{
class SerialSet : public std::set<unsigned int>
{
public:
  SerialSet( Clib::ConfigElem& elem, const char* tag );
  SerialSet() {}
  void writeOn( Clib::StreamWriter& sw, const char* tag ) const;
};

class Guild : public ref_counted
{
public:
  explicit Guild( Clib::ConfigElem& elem );
  explicit Guild( unsigned int guildid );

  unsigned int guildid() const;
  bool disbanded() const;
  bool hasMembers() const;
  bool hasAllies() const;
  bool hasEnemies() const;

  bool hasAlly( const Guild* g2 ) const;
  bool hasEnemy( const Guild* g2 ) const;

  void disband();

  void update_online_members();
  static void update_online_members_remove( Mobile::Character* chr );

  void printOn( Clib::StreamWriter& sw ) const;
  void addMember( unsigned int serial );
  std::vector<Mobile::Character*> get_members( bool include_offline = false ) const;

  void registerWithMembers();
  size_t estimateSize() const;

  static bool AreAllies( Guild* g1, Guild* g2 );
  static bool AreEnemies( Guild* g1, Guild* g2 );
  static Guild* FindGuild( unsigned int guildid );
  static Guild* FindOrCreateGuild( unsigned int guildid, unsigned int memberserial );

  friend class Module::EGuildRefObjImp;

private:
  unsigned int _guildid;
  SerialSet _member_serials;
  SerialSet _allyguild_serials;
  SerialSet _enemyguild_serials;
  PropertyList _proplist;
  bool _disbanded;
};
}  // namespace Core
}  // namespace Pol
#endif
