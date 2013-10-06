/*
History
=======


Notes
=======

*/

#ifndef GUILDS_H

class BObjectImp;

#include "../../bscript/execmodl.h"

class SerialSet : public std::set<unsigned int>
{
public:
    SerialSet( ConfigElem& elem, const char* tag );
    SerialSet() {}

    void writeOn( StreamWriter& sw, const char* tag ) const;

};

class Guild : public ref_counted
{
public:
    explicit Guild( ConfigElem& elem );
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
	static void update_online_members_remove( Character* chr );

    void printOn( StreamWriter& sw ) const;
    void addMember( unsigned int serial );

    void registerWithMembers();

    friend class EGuildRefObjImp;

private:
    unsigned int _guildid;
    SerialSet _member_serials;
    SerialSet _allyguild_serials;
    SerialSet _enemyguild_serials;
    PropertyList _proplist;
    bool _disbanded;
};

class GuildExecutorModule : public TmplExecutorModule<GuildExecutorModule>
{
public:
	GuildExecutorModule( Executor& exec ) :
	TmplExecutorModule<GuildExecutorModule>( "Guilds", exec ) {};

	BObjectImp* mf_ListGuilds();
	BObjectImp* mf_CreateGuild();
	BObjectImp* mf_FindGuild();
	BObjectImp* mf_DestroyGuild();
};

Guild* FindOrCreateGuild( unsigned int guildid, unsigned int memberserial );
BObjectImp* CreateGuildRefObjImp( Guild* guild );

bool AreAllies( Guild* g1, Guild* g2 );
bool AreEnemies( Guild* g1, Guild* g2 );

#endif
