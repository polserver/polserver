/*
History
=======


Notes
=======

*/

#ifndef GUILDS_H
#define GUILDS_H

#include "../../bscript/execmodl.h"
#include "../proplist.h"

namespace Pol {
    namespace Clib {
        class ConfigElem;
        class StreamWriter;
    }
    namespace Mobile {
        class Character;
    }

  namespace Module {
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

	  void registerWithMembers();

	  friend class EGuildRefObjImp;

	private:
	  unsigned int _guildid;
	  SerialSet _member_serials;
	  SerialSet _allyguild_serials;
	  SerialSet _enemyguild_serials;
	  Core::PropertyList _proplist;
	  bool _disbanded;
	};

	class GuildExecutorModule : public Bscript::TmplExecutorModule<GuildExecutorModule>
	{
	public:
	  GuildExecutorModule( Bscript::Executor& exec ) :
		TmplExecutorModule<GuildExecutorModule>( "Guilds", exec ) {};

	  Bscript::BObjectImp* mf_ListGuilds( );
	  Bscript::BObjectImp* mf_CreateGuild( );
	  Bscript::BObjectImp* mf_FindGuild( );
	  Bscript::BObjectImp* mf_DestroyGuild( );
	};

	Guild* FindOrCreateGuild( unsigned int guildid, unsigned int memberserial );
	Bscript::BObjectImp* CreateGuildRefObjImp( Guild* guild );

	bool AreAllies( Guild* g1, Guild* g2 );
	bool AreEnemies( Guild* g1, Guild* g2 );
  }
}
#endif
