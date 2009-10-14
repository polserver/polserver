/*
History
=======


Notes
=======

*/

#ifndef GUILDS_H

class BObjectImp;

#include "../../bscript/execmodl.h"

class SerialSet : public std::set<unsigned long>
{
public:
    SerialSet( ConfigElem& elem, const char* tag );
    SerialSet() {}

    void writeOn( ostream& os, const char* tag ) const;

};

class Guild : public ref_counted
{
public:
    explicit Guild( ConfigElem& elem );
    explicit Guild( unsigned long guildid );

    unsigned long guildid() const;
    bool disbanded() const;
    bool hasMembers() const;
    bool hasAllies() const;
    bool hasEnemies() const;

    bool hasAlly( const Guild* g2 ) const;
    bool hasEnemy( const Guild* g2 ) const;

    void disband();

	void update_online_members();
	void update_online_members_remove( Character* chr );

    void printOn( ostream& os ) const;
    void addMember( unsigned long serial );

    void registerWithMembers();

    friend class EGuildRefObjImp;

private:
    unsigned long _guildid;
    SerialSet _member_serials;
    SerialSet _allyguild_serials;
    SerialSet _enemyguild_serials;
    PropertyList _proplist;
    bool _disbanded;

    //typedef std::vector< ref_ptr<Character> > Members;
    // Members members;

    //typedef std::vector< ref_ptr<Guild> > GuildVec;
    //GuildVec _allyguilds;
    //GuildVec _enemyguilds;
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

Guild* FindOrCreateGuild( unsigned long guildid, unsigned long memberserial );
BObjectImp* CreateGuildRefObjImp( Guild* guild );

bool AreAllies( Guild* g1, Guild* g2 );
bool AreEnemies( Guild* g1, Guild* g2 );

#endif
