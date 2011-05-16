/*
History
=======

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include "../../clib/cfgelem.h"
#include "../../clib/maputil.h"
#include "../../clib/passert.h"

#include "attribute.h"
#include "../../plib/pkg.h"
#include "../syshook.h"
#include "../ssopt.h"

std::vector< Attribute* > attributes;
unsigned numAttributes;
typedef std::map< std::string, Attribute*, ci_cmp_pred > AttributesByName;
AttributesByName attributes_byname;

const Attribute* pAttrStrength;
const Attribute* pAttrIntelligence;
const Attribute* pAttrDexterity;
const Attribute* pAttrParry;
const Attribute* pAttrTactics;

Attribute* FindAttribute( const std::string& str )
{
	AttributesByName::const_iterator citr = attributes_byname.find( str );
	if (citr != attributes_byname.end())
		return (*citr).second;
	else
		return NULL;
}

Attribute* FindAttribute( unsigned attrid )
{
	if (attrid < attributes.size())
		return attributes[attrid];
	else
		return NULL;
}

Attribute::Attribute( const Package* pkg, ConfigElem& elem ) :
	pkg(pkg),
	name( elem.rest() ),
	attrid(0),
	aliases(),
	next(NULL),
	getintrinsicmod_func(NULL),
	delay_seconds(elem.remove_ushort("DELAY", 0)),
	unhides(elem.remove_bool("UNHIDES", true)),
	disable_core_checks(elem.remove_bool("DisableCoreChecks", false)),
	default_cap(elem.remove_ushort("DefaultCap", ssopt.default_attribute_cap)),
	script_( elem.remove_string("SCRIPT", ""), pkg, "scripts/skills/")
{
	aliases.push_back( name );
	string tmp;
	while (elem.remove_prop( "Alias", &tmp ))
		aliases.push_back( tmp );

	if (elem.remove_prop( "GetIntrinsicModFunction", &tmp ))
	{
		getintrinsicmod_func = FindExportedFunction( elem, pkg, tmp, 1 );
	}
}

void load_attribute_entry( const Package* pkg, ConfigElem& elem )
{
	Attribute* attr = new Attribute( pkg, elem );
	
	const Attribute* existing = FindAttribute( attr->name );
	if (existing)
	{
		elem.throw_error( "Attribute " + attr->name + " already defined by "
				+ existing->pkg->desc() );
	}
	attr->attrid = attributes.size();
	if (!attributes.empty())
		attributes.back()->next = attr;
	attributes.push_back( attr );
	for( unsigned i = 0; i < attr->aliases.size(); ++i )
	{
		attributes_byname[ attr->aliases[i] ] = attr;
	}
}

void load_attributes_cfg()
{
	load_packaged_cfgs( "attributes.cfg", "Attribute", load_attribute_entry );
	numAttributes = attributes.size();

	pAttrStrength = FindAttribute( "Strength" );
	pAttrIntelligence = FindAttribute( "Intelligence" );
	pAttrDexterity = FindAttribute( "Dexterity" );

	pAttrParry = FindAttribute( "Parry" );
	pAttrTactics = FindAttribute( "Tactics" );

	passert_always( pAttrStrength );
	passert_always( pAttrIntelligence );
	passert_always( pAttrDexterity );
	passert_always( pAttrParry );
	passert_always( pAttrTactics );
}

void clean_attributes()
{
	std::vector<Attribute*>::iterator iter = attributes.begin();
	for ( ; iter != attributes.end(); ++iter) {
		delete *iter;
		*iter=NULL;
	}
	attributes.clear();
	attributes_byname.clear();
}

