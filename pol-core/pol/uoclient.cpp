/** @file
 *
 * @par History
 * - 2010/02/03 Turley:    MethodScript support for mobiles
 */


#include "uoclient.h"

#include <string>

#include "../clib/cfgelem.h"
#include "../clib/logfacility.h"
#include "../plib/pkg.h"
#include "globals/network.h"
#include "mobile/attribute.h"
#include "skillid.h"
#include "syshookscript.h"
#include "vital.h"


namespace Pol
{
namespace Core
{
UoClientProtocol::UoClientProtocol() : EnableFlowControlPackets( false ) {}
size_t UoClientProtocol::estimateSize() const
{
  return sizeof( UoClientProtocol );
}


UoClientListener::UoClientListener( Clib::ConfigElem& elem )
    : port( elem.remove_ushort( "PORT" ) ),
      aosresist( elem.remove_bool( "AOSRESISTANCES", false ) ),
      sticky( elem.remove_bool( "KeepClients", false ) ),
      login_clients_size( 0 ),
      login_clients()

{
  CalculateCryptKeys( elem.remove_string( "ENCRYPTION", "none" ), encryption );
}

size_t UoClientListener::estimateSize() const
{
  size_t size = sizeof( UoClientListener );
  size += login_clients_size * ( sizeof( UoClientThread ) + 3 * sizeof( void* ) );
  return size;
}

void checka( Clib::ConfigElem& elem, UoClientGeneral::Mapping& mapping, const char* tag )
{
  if ( !mapping.any )
  {
    std::string name;
    if ( elem.remove_prop( tag, &name ) )
    {
      mapping.name = name;
      mapping.any = true;
      Mobile::Attribute* pAttr = Mobile::Attribute::FindAttribute( name );
      if ( pAttr )
        mapping.id = pAttr->attrid;
      else
        elem.throw_error( "Attribute " + name + " not found" );
    }
  }
}
void checkv( Clib::ConfigElem& elem, UoClientGeneral::Mapping& mapping, const char* tag )
{
  if ( !mapping.any )
  {
    std::string name;
    if ( elem.remove_prop( tag, &name ) )
    {
      mapping.name = name;
      mapping.any = true;
      Vital* pVital = FindVital( name );
      if ( pVital )
        mapping.id = pVital->vitalid;
      else
        elem.throw_error( "Vital " + name + " not found" );
    }
  }
}

// Strength
// Dexterity
void load_general_entry( const Plib::Package* pkg, Clib::ConfigElem& elem )
{
  checka( elem, networkManager.uoclient_general.strength, "Strength" );
  checka( elem, networkManager.uoclient_general.intelligence, "Intelligence" );
  checka( elem, networkManager.uoclient_general.dexterity, "Dexterity" );
  checkv( elem, networkManager.uoclient_general.hits, "Hits" );
  checkv( elem, networkManager.uoclient_general.stamina, "Stamina" );
  checkv( elem, networkManager.uoclient_general.mana, "Mana" );
  // dave changed 3/15/03, support configurable max skillid
  networkManager.uoclient_general.maxskills = elem.remove_ushort( "MaxSkillID", SKILLID__HIGHEST );
  std::string temp;
  if ( elem.remove_prop( "MethodScript", &temp ) )
  {
    if ( !temp.empty() )
    {
      ExportScript* shs = new ExportScript( pkg, temp );
      if ( shs->Initialize() )
      {
        networkManager.uoclient_general.method_script = shs;
        POLLOG_INFO << "\n"
                       "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
                       "WARNING: uoclient.cfg MethodScript entry is deprecated! Use syshook.cfg "
                       "SystemMethod Mobile entry instead.\n"
                       "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
      }
      else
        delete shs;
    }
  }
}

void load_protocol_entry( const Plib::Package* /*pkg*/, Clib::ConfigElem& elem )
{
  networkManager.uoclient_protocol.EnableFlowControlPackets =
      elem.remove_bool( "EnableFlowControlPackets" );
}

void load_listener_entry( const Plib::Package* /*pkg*/, Clib::ConfigElem& elem )
{
  networkManager.uoclient_listeners.emplace_back( elem );
}

void load_uoclient_entry( const Plib::Package* pkg, Clib::ConfigElem& elem )
{
  if ( elem.type_is( "general" ) )
    load_general_entry( pkg, elem );
  else if ( elem.type_is( "protocol" ) )
    load_protocol_entry( pkg, elem );
  else if ( elem.type_is( "listener" ) )
    load_listener_entry( pkg, elem );
}

void load_uoclient_cfg()
{
  load_packaged_cfgs( "uoclient.cfg", "general protocol listener", load_uoclient_entry );
}

size_t UoClientGeneral::Mapping::estimateSize() const
{
  return sizeof( bool ) + name.capacity() + sizeof( unsigned );
}
size_t UoClientGeneral::estimateSize() const
{
  size_t size = strength.estimateSize() + intelligence.estimateSize() + dexterity.estimateSize() +
                hits.estimateSize() + stamina.estimateSize() + mana.estimateSize() +
                sizeof( unsigned short ) /*maxskills*/
                + sizeof( ExportScript* );
  if ( method_script != nullptr )
    size += method_script->estimateSize();
  return size;
}

void UoClientGeneral::deinitialize()
{
  if ( method_script != nullptr )
  {
    delete method_script;
    method_script = nullptr;
  }
}
}  // namespace Core
}  // namespace Pol
