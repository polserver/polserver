/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"

#include "clib/cfgelem.h"

#include "attribute.h"
#include "plib/pkg.h"
#include "skillid.h"
#include "uoclient.h"
#include "vital.h"


UoClientGeneral uoclient_general;
UoClientProtocol uoclient_protocol;
UoClientListeners uoclient_listeners;

UoClientListener::UoClientListener( ConfigElem& elem ) :
    port( elem.remove_ushort( "PORT" ) ),
    encryption( elem.remove_string( "ENCRYPTION" ) ),
	aosresist( elem.remove_bool( "AOSRESISTANCES", false ) )
{

}

void checka( ConfigElem& elem, UoClientGeneral::Mapping& mapping, const char* tag )
{
    if (!mapping.any)
    {
        string name;
        if (elem.remove_prop( tag, &name ))
        {
            mapping.name = name;
            mapping.any = true;
            Attribute* pAttr = FindAttribute( name );
            if (pAttr)
                mapping.id = pAttr->attrid;
            else
                elem.throw_error( "Attribute " + name + " not found" );
        }
    }
}
void checkv( ConfigElem& elem, UoClientGeneral::Mapping& mapping, const char* tag )
{
    if (!mapping.any)
    {
        string name;
        if (elem.remove_prop( tag, &name ))
        {
            mapping.name = name;
            mapping.any = true;
            Vital* pVital = FindVital( name );
            if (pVital)
                mapping.id = pVital->vitalid;
            else
                elem.throw_error( "Vital " + name + " not found" );
        }
    }
}

// Strength
// Dexterity
void load_general_entry( const Package* pkg, ConfigElem& elem )
{
    checka( elem, 
           uoclient_general.strength, 
           "Strength" );
    checka( elem, 
           uoclient_general.intelligence, 
           "Intelligence" );
    checka( elem, 
           uoclient_general.dexterity, 
           "Dexterity" );
    checkv( elem, 
            uoclient_general.hits, 
            "Hits" );
    checkv( elem, 
            uoclient_general.stamina, 
            "Stamina" );
    checkv( elem, 
            uoclient_general.mana, 
            "Mana" );
	//dave changed 3/15/03, support configurable max skillid
	uoclient_general.maxskills = elem.remove_ushort("MaxSkillID",SKILLID__HIGHEST);
}

void load_protocol_entry( const Package* pkg, ConfigElem& elem )
{
    uoclient_protocol.EnableFlowControlPackets = elem.remove_bool( "EnableFlowControlPackets" );
}

void load_listener_entry( const Package* pkg, ConfigElem& elem )
{
    uoclient_listeners.push_back( UoClientListener( elem ) );
}

void load_uoclient_entry( const Package* pkg, ConfigElem& elem )
{
    if (elem.type_is( "general" ))
        load_general_entry( pkg, elem );
    else if (elem.type_is( "protocol" ))
        load_protocol_entry( pkg, elem );
    else if (elem.type_is( "listener" ))
        load_listener_entry( pkg, elem );
}

void load_uoclient_cfg()
{
    load_packaged_cfgs( "uoclient.cfg",
                        "general protocol listener",
                        load_uoclient_entry );
}
