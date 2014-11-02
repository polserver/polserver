/*
History
=======
2005/09/14 Shinigami: regen_while_dead implemented

Notes
=======

*/

#include "vital.h"

//nclude "charactr.h"
#include "checkpnt.h"
#include "syshook.h"

#include "../plib/pkg.h"

#include "../clib/cfgelem.h"
#include "../clib/maputil.h"
#include "../clib/passert.h"

#include <vector>
#include <string>
#include <iostream>
#include <map>

namespace Pol {
  namespace Core {
	std::vector< Vital* > vitals;
	unsigned numVitals;

    typedef std::map< std::string, Vital*, Clib::ci_cmp_pred > VitalsByName;
	VitalsByName vitals_byname;

	const Vital* pVitalLife;
	const Vital* pVitalStamina;
	const Vital* pVitalMana;

    Vital::Vital( const Plib::Package* pkg, Clib::ConfigElem& elem ) :
	  pkg( pkg ),
	  name( elem.rest() ),
	  aliases(),
	  vitalid( 0 ),
	  next( NULL ),
	  get_regenrate_func( FindExportedFunction( elem, pkg, elem.remove_string( "RegenRateFunction" ), 1 ) ),
	  get_maximum_func( FindExportedFunction( elem, pkg, elem.remove_string( "MaximumFunction" ), 1 ) ),
	  underflow_func( NULL ),
	  regen_while_dead( elem.remove_bool( "RegenWhileDead", false ) )
	{
	  aliases.push_back( name );
      std::string tmp;
	  while ( elem.remove_prop( "Alias", &tmp ) )
		aliases.push_back( tmp );

	  if ( elem.remove_prop( "UnderflowFunction", &tmp ) )
	  {
		underflow_func = FindExportedFunction( elem, pkg, tmp, 2 );
	  }
	}

	Vital::~Vital()
	{
	  if ( this->underflow_func != NULL )
		delete this->underflow_func;

	  if ( this->get_maximum_func != NULL )
		delete this->get_maximum_func;

	  if ( this->get_regenrate_func != NULL )
		delete this->get_regenrate_func;

	  this->get_maximum_func = NULL;
	  this->underflow_func = NULL;
	  this->get_regenrate_func = NULL;
	}
	void clean_vitals()
	{
	  std::vector<Vital*>::iterator iter = vitals.begin();
	  for ( ; iter != vitals.end(); ++iter )
	  {
		delete *iter;
		*iter = NULL;
	  }
	  vitals.clear();
	  vitals_byname.clear();
	}

	Vital* FindVital( const std::string& str )
	{
	  VitalsByName::const_iterator citr = vitals_byname.find( str );
	  if ( citr != vitals_byname.end() )
		return ( *citr ).second;
	  else
		return NULL;
	}

	Vital* FindVital( unsigned vitalid )
	{
	  if ( vitalid < vitals.size() )
		return vitals[vitalid];
	  else
		return NULL;
	}

    void load_vital_entry( const Plib::Package* pkg, Clib::ConfigElem& elem )
	{
	  Vital* vital = new Vital( pkg, elem );

	  const Vital* existing = FindVital( vital->name );
	  if ( existing )
	  {
		elem.throw_error( "Vital " + vital->name + " already defined by "
						  + existing->pkg->desc() );
	  }
	  vital->vitalid = static_cast<unsigned int>( vitals.size() );
	  if ( !vitals.empty() )
		vitals.back()->next = vital;
	  vitals.push_back( vital );

	  for ( unsigned i = 0; i < vital->aliases.size(); ++i )
	  {
		vitals_byname[vital->aliases[i]] = vital;
	  }
	}

	void load_vitals_cfg()
	{
	  checkpoint( "load_vitals_cfg: load_packed_cfgs" );
	  load_packaged_cfgs( "vitals.cfg", "Vital", load_vital_entry );

	  numVitals = static_cast<unsigned int>( vitals.size() );

	  checkpoint( "load_vitals_cfg: find Life vital" );
	  pVitalLife = FindVital( "Life" );
	  checkpoint( "load_vitals_cfg: find Stamina vital" );
	  pVitalStamina = FindVital( "Stamina" );
	  checkpoint( "load_vitals_cfg: find Mana vital" );
	  pVitalMana = FindVital( "Mana" );

	  checkpoint( "load_vitals_cfg: verify vital vitals" );
	  passert_always( pVitalLife );
	  passert_always( pVitalStamina );
	  passert_always( pVitalMana );
	}
  }
}