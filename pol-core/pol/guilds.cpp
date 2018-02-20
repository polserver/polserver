
#include "guilds.h"

#include <stddef.h>
#include <string>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/cfgsect.h"
#include "../clib/fileutil.h"
#include "../clib/streamsaver.h"
#include "../plib/systemstate.h"
#include "fnsearch.h"
#include "globals/uvars.h"
#include "mobile/charactr.h"
#include "ufunc.h"
#include "uobject.h"

namespace Pol
{
namespace Core
{
SerialSet::SerialSet( Clib::ConfigElem& elem, const char* tag )
{
  unsigned int tmp;
  while ( elem.remove_prop( tag, &tmp ) )
  {
    insert( tmp );
  }
}

void SerialSet::writeOn( Clib::StreamWriter& os, const char* tag ) const
{
  for ( const_iterator citr = begin(), citrend = end(); citr != citrend; ++citr )
  {
    os() << "\t" << tag << "\t0x" << fmt::hex( *citr ) << pf_endl;
  }
}

Guild::Guild( Clib::ConfigElem& elem )
    : _guildid( elem.remove_ushort( "GUILDID" ) ),
      _member_serials( elem, "MEMBER" ),
      _allyguild_serials( elem, "ALLYGUILD" ),
      _enemyguild_serials( elem, "ENEMYGUILD" ),
      _proplist( Core::CPropProfiler::Type::GUILD ),
      _disbanded( false )
{
  _proplist.readProperties( elem );
}

Guild::Guild( unsigned int guildid )
    : _guildid( guildid ),
      _member_serials(),
      _allyguild_serials(),
      _enemyguild_serials(),
      _proplist( Core::CPropProfiler::Type::GUILD ),
      _disbanded( false )
{
  if ( _guildid >= Core::gamestate.nextguildid )
    Core::gamestate.nextguildid = _guildid + 1;
}

void Guild::registerWithMembers()
{
  for ( SerialSet::iterator itr = _member_serials.begin(); itr != _member_serials.end(); /* */ )
  {
    unsigned int mserial = ( *itr );
    SerialSet::iterator last_itr = itr;
    ++itr;

    Mobile::Character* chr = system_find_mobile( mserial );
    if ( chr != NULL )
    {
      chr->guild( this );
    }
    else
    {
      _member_serials.erase( last_itr );
    }
  }
}

void Guild::update_online_members()
{
  // FIXME : All of guilds.cpp iterator's need rewritten. Turley found a much better method
  // used in the party system.
  // NOTE: stlport seems to not return save itr on erase, but with a list container the iterator
  // should stay valid
  for ( SerialSet::iterator itr = _member_serials.begin(), end = _member_serials.end(); itr != end;
        /* */ )
  {
    unsigned int mserial = ( *itr );
    SerialSet::iterator last_itr = itr;
    ++itr;

    Mobile::Character* chr = Core::system_find_mobile( mserial );
    if ( chr != NULL )
    {
      if ( chr->client )
      {
        send_move( chr->client, chr );
        send_remove_character_to_nearby_cansee( chr );
        send_create_mobile_to_nearby_cansee( chr );
      }
    }
    else
    {
      _member_serials.erase( last_itr );
    }
  }
}

void Guild::update_online_members_remove( Mobile::Character* chr )
{
  if ( chr->client )
  {
    send_move( chr->client, chr );
    send_remove_character_to_nearby_cansee( chr );
    send_create_mobile_to_nearby_cansee( chr );
  }
}

unsigned int Guild::guildid() const
{
  return _guildid;
}

bool Guild::disbanded() const
{
  return _disbanded;
}

void Guild::disband()
{
  _disbanded = true;
}

bool Guild::hasMembers() const
{
  return !_member_serials.empty();
}

bool Guild::hasAllies() const
{
  return !_allyguild_serials.empty();
}
bool Guild::hasAlly( const Guild* g2 ) const
{
  return _allyguild_serials.count( g2->guildid() ) != 0;
}

bool Guild::hasEnemies() const
{
  return !_enemyguild_serials.empty();
}
bool Guild::hasEnemy( const Guild* g2 ) const
{
  return _enemyguild_serials.count( g2->guildid() ) != 0;
}


void Guild::printOn( Clib::StreamWriter& sw ) const
{
  sw() << "Guild" << pf_endl << "{" << pf_endl << "\tGuildId\t" << _guildid << pf_endl;
  _member_serials.writeOn( sw, "Member" );
  _allyguild_serials.writeOn( sw, "AllyGuild" );
  _enemyguild_serials.writeOn( sw, "EnemyGuild" );
  _proplist.printProperties( sw );
  sw() << "}" << pf_endl << pf_endl;
  // sw.flush();
}

void Guild::addMember( unsigned int serial )
{
  _member_serials.insert( serial );
}

bool Guild::AreAllies( Guild* g1, Guild* g2 )
{
  return ( g1 == g2 || g1->hasAlly( g2 ) );
}
bool Guild::AreEnemies( Guild* g1, Guild* g2 )
{
  return g1->hasEnemy( g2 );
}

void register_guilds()
{
  for ( Guilds::iterator itr = Core::gamestate.guilds.begin(); itr != Core::gamestate.guilds.end();
        ++itr )
  {
    Guild* guild = ( *itr ).second.get();
    guild->registerWithMembers();
  }
}

void read_guilds_dat()
{
  std::string guildsfile = Plib::systemstate.config.world_data_path + "guilds.txt";

  if ( !Clib::FileExists( guildsfile ) )
    return;

  Clib::ConfigFile cf( guildsfile );
  Clib::ConfigSection sect_general( cf, "GENERAL", Clib::CST_UNIQUE );
  Clib::ConfigSection sect_guild( cf, "GUILD", Clib::CST_NORMAL );
  Clib::ConfigElem elem;
  while ( cf.read( elem ) )
  {
    if ( sect_general.matches( elem ) )
    {
      Core::gamestate.nextguildid = elem.remove_ulong( "NEXTGUILDID", 1 );
    }
    else if ( sect_guild.matches( elem ) )
    {
      Guild* guild = new Guild( elem );
      Core::gamestate.guilds[guild->guildid()].set( guild );
    }
  }

  register_guilds();
}

void write_guilds( Clib::StreamWriter& sw )
{
  sw() << "General" << pf_endl << "{" << pf_endl << "\tNextGuildId\t" << Core::gamestate.nextguildid
       << pf_endl << "}" << pf_endl << pf_endl;

  for ( const auto& _guild : Core::gamestate.guilds )
  {
    const Guild* guild = _guild.second.get();
    guild->printOn( sw );
  }
}

Guild* Guild::FindOrCreateGuild( unsigned int guildid, unsigned int memberserial )
{
  Guilds::iterator itr = Core::gamestate.guilds.find( guildid );
  Guild* guild = NULL;
  if ( itr != Core::gamestate.guilds.end() )
    guild = ( *itr ).second.get();
  else
  {
    guild = new Guild( guildid );
    Core::gamestate.guilds[guildid].set( guild );
  }

  // this is called from Character::readCommonProperties, which assigns
  // chr->guild (so we don't do it here)
  guild->addMember( memberserial );

  return guild;
}
Guild* Guild::FindGuild( unsigned int guildid )
{
  Guilds::iterator itr = Core::gamestate.guilds.find( guildid );
  Guild* guild = NULL;

  if ( itr != Core::gamestate.guilds.end() )
    guild = ( *itr ).second.get();

  return guild;
}

size_t Guild::estimateSize() const
{
  return sizeof( unsigned int ) /*_guildid*/
         + 3 * sizeof( void* ) +
         _member_serials.size() * ( sizeof( unsigned int ) + 3 * sizeof( void* ) ) +
         3 * sizeof( void* ) +
         _allyguild_serials.size() * ( sizeof( unsigned int ) + 3 * sizeof( void* ) ) +
         3 * sizeof( void* ) +
         _enemyguild_serials.size() * ( sizeof( unsigned int ) + 3 * sizeof( void* ) ) +
         _proplist.estimatedSize() + sizeof( bool ); /*_disbanded*/
}
}
}
