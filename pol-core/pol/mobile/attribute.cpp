/*
History
=======

Notes
=======

*/

#include "attribute.h"

#include "../../clib/cfgelem.h"
#include "../../clib/maputil.h"
#include "../../clib/passert.h"

#include "../../plib/pkg.h"
#include "../syshook.h"
#include "../globals/uvars.h"
#include "../globals/settings.h"

namespace Pol {
  namespace Mobile {

	Attribute* Attribute::FindAttribute( const std::string& str )
	{
	  Core::AttributesByName::const_iterator citr = Core::gamestate.attributes_byname.find( str );
	  if ( citr != Core::gamestate.attributes_byname.end() )
		return ( *citr ).second;
	  else
		return NULL;
	}

	Attribute* Attribute::FindAttribute( unsigned attrid )
	{
	  if ( attrid < Core::gamestate.attributes.size() )
		return Core::gamestate.attributes[attrid];
	  else
		return NULL;
	}

	Attribute::Attribute( const Plib::Package* pkg, Clib::ConfigElem& elem ) :
	  pkg( pkg ),
	  name( elem.rest() ),
	  attrid( 0 ),
	  aliases(),
	  next( NULL ),
	  getintrinsicmod_func( nullptr ),
	  delay_seconds( elem.remove_ushort( "DELAY", 0 ) ),
	  unhides( elem.remove_bool( "UNHIDES", true ) ),
	  disable_core_checks( elem.remove_bool( "DisableCoreChecks", false ) ),
	  default_cap( elem.remove_ushort( "DefaultCap", Core::settingsManager.ssopt.default_attribute_cap ) ),
	  script_( elem.remove_string( "SCRIPT", "" ), pkg, "scripts/skills/" )
	{
	  aliases.push_back( name );
	  std::string tmp;
	  while ( elem.remove_prop( "Alias", &tmp ) )
		aliases.push_back( tmp );

	  if ( elem.remove_prop( "GetIntrinsicModFunction", &tmp ) )
	  {
		getintrinsicmod_func = Core::FindExportedFunction( elem, pkg, tmp, 1 );
	  }
	}
	Attribute::~Attribute()
	{
	  if (getintrinsicmod_func != nullptr)
		delete getintrinsicmod_func;
	}

	void load_attribute_entry( const Plib::Package* pkg, Clib::ConfigElem& elem )
	{
	  auto attr = new Attribute( pkg, elem );

	  const Attribute* existing = Attribute::FindAttribute( attr->name );
	  if ( existing )
	  {
		elem.throw_error( "Attribute " + attr->name + " already defined by "
						  + existing->pkg->desc() );
	  }
	  attr->attrid = static_cast<unsigned int>( Core::gamestate.attributes.size() );
	  if ( !Core::gamestate.attributes.empty() )
		Core::gamestate.attributes.back()->next = attr;
	  Core::gamestate.attributes.push_back( attr );
	  for ( unsigned i = 0; i < attr->aliases.size(); ++i )
	  {
		Core::gamestate.attributes_byname[attr->aliases[i]] = attr;
	  }
	}

	void load_attributes_cfg()
	{
	  load_packaged_cfgs( "attributes.cfg", "Attribute", load_attribute_entry );
	  Core::gamestate.numAttributes = static_cast<unsigned int>( Core::gamestate.attributes.size() );

	  Core::gamestate.pAttrStrength = Attribute::FindAttribute( "Strength" );
	  Core::gamestate.pAttrIntelligence = Attribute::FindAttribute( "Intelligence" );
	  Core::gamestate.pAttrDexterity = Attribute::FindAttribute( "Dexterity" );

	  Core::gamestate.pAttrParry = Attribute::FindAttribute( "Parry" );
	  Core::gamestate.pAttrTactics = Attribute::FindAttribute( "Tactics" );

	  passert_always( Core::gamestate.pAttrStrength );
	  passert_always( Core::gamestate.pAttrIntelligence );
	  passert_always( Core::gamestate.pAttrDexterity );
	  passert_always( Core::gamestate.pAttrParry );
	  passert_always( Core::gamestate.pAttrTactics );
	}

	void clean_attributes()
	{
	  auto iter = Core::gamestate.attributes.begin();
	  for ( ; iter != Core::gamestate.attributes.end(); ++iter )
	  {
		delete *iter;
		*iter = NULL;
	  }
	  Core::gamestate.attributes.clear();
	  Core::gamestate.attributes_byname.clear();
	}

  }
}