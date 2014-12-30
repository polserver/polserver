/*
History
=======
2010/02/03 Turley:    MethodScript support for mobiles


Notes
=======

*/

#include "uoclient.h"

#include "mobile/attribute.h"
#include "skillid.h"
#include "vital.h"
#include "syshookscript.h"
#include "uvars.h"

#include "crypt/cryptkey.h"

#include "../plib/pkg.h"
#include "../clib/cfgelem.h"


namespace Pol {
  namespace Core {
	UoClientProtocol::UoClientProtocol() :
	  EnableFlowControlPackets(false)
	  {}


	UoClientListener::UoClientListener( Clib::ConfigElem& elem ) :
	  port( elem.remove_ushort( "PORT" ) ),
	  aosresist( elem.remove_bool( "AOSRESISTANCES", false ) ),
	  sticky( elem.remove_bool( "KeepClients", false ) )

	{
	  CalculateCryptKeys( elem.remove_string( "ENCRYPTION", "none" ), encryption );
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
	  checka( elem,
			  gamestate.uoclient_general.strength,
			  "Strength" );
	  checka( elem,
			  gamestate.uoclient_general.intelligence,
			  "Intelligence" );
	  checka( elem,
			  gamestate.uoclient_general.dexterity,
			  "Dexterity" );
	  checkv( elem,
			  gamestate.uoclient_general.hits,
			  "Hits" );
	  checkv( elem,
			  gamestate.uoclient_general.stamina,
			  "Stamina" );
	  checkv( elem,
			  gamestate.uoclient_general.mana,
			  "Mana" );
	  //dave changed 3/15/03, support configurable max skillid
	  gamestate.uoclient_general.maxskills = elem.remove_ushort( "MaxSkillID", SKILLID__HIGHEST );
	  std::string temp;
	  if ( elem.remove_prop( "MethodScript", &temp ) )
	  {
		if ( !temp.empty() )
		{
		  ExportScript* shs = new ExportScript( pkg, temp );
		  if ( shs->Initialize() )
			gamestate.uoclient_general.method_script = shs;
		  else
			delete shs;
		}
	  }
	}

    void load_protocol_entry( const Plib::Package* /*pkg*/, Clib::ConfigElem& elem )
	{
	  gamestate.uoclient_protocol.EnableFlowControlPackets = elem.remove_bool( "EnableFlowControlPackets" );
	}

    void load_listener_entry( const Plib::Package* /*pkg*/, Clib::ConfigElem& elem )
	{
	  gamestate.uoclient_listeners.push_back( UoClientListener( elem ) );
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
	  load_packaged_cfgs( "uoclient.cfg",
						  "general protocol listener",
						  load_uoclient_entry );
	}

	UoClientGeneral::~UoClientGeneral()
	{
	  if ( method_script != NULL )
	  {
		delete method_script;
		method_script = NULL;
	  }
	}
  }
}