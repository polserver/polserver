/*
History
=======
2007/06/17 Shinigami: added config.world_data_path

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/impstr.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfile.h"
#include "../clib/passert.h"
#include "../clib/random.h"
#include "../clib/stlutil.h"

#include "../plib/maptile.h"
#include "../plib/realm.h"

#include "item/itemdesc.h"
#include "polcfg.h"
#include "polclock.h"
#include "polsig.h"
#include "proplist.h"
#include "realms.h"
#include "resource.h"
#include "schedule.h"
#include "udatfile.h"
#include "uofile.h"

///$TITLE=Resource Management

ResourceRegion::ResourceRegion(ConfigElem& elem, RegionId id) : Region(elem, id),
	tilecount_(0),

	units_per_area_(elem.remove_ulong( "UnitsPerArea" )),
	seconds_per_regrow_(elem.remove_ulong( "SecondsPerRegrow" )),
	last_regen_(poltime()),

	capacity_(elem.remove_ulong( "Capacity" )),
	units_(0)

{
}


void ResourceRegion::read_data( ConfigElem& elem )
{
	units_ = elem.remove_ulong( "Units" );
}


/// [1] Harvest Difficulty
///	  
BObjectImp* ResourceRegion::get_harvest_difficulty( xcoord x, ycoord y, Realm* realm )
{
	unsigned int xy = (x<<16)|y;
	Depletions::iterator itr = depletions_.find( xy );
	int units = units_per_area_;
	if (itr != depletions_.end())
	{
		units -= (*itr).second;
	}
	passert( units >= 0 );
	
	if (units == 0)
	{
		return new BError( "Resource is depleted" );
	}
	
	unsigned int base_difficulty = 90 - (units * 80 / units_per_area_);
	unsigned int this_difficulty = random_int( base_difficulty * 2 );
	return new BLong( this_difficulty );
}

BObjectImp* ResourceRegion::harvest_resource( xcoord x, ycoord y, int n, int m )
{
	unsigned int xy = (x<<16)|y;
	Depletions::iterator itr = depletions_.find( xy );
	int avail_units = units_per_area_;
	if (itr != depletions_.end())
	{
		avail_units -= (*itr).second;
	}
	passert( avail_units >= 0 );
	int a = avail_units / n;
	if (a > m) 
		a = m;
	short na = static_cast<short>(n*a);
	if (itr != depletions_.end())
	{
		(*itr).second += na;
	}
	else
	{
		depletions_[xy] = na;
	}
	return new BLong( na );
}

void ResourceRegion::regenerate( time_t now )
{
	unsigned int diff = static_cast<unsigned int>(now - last_regen_);
	unsigned int n_regrow;
	if (seconds_per_regrow_)
	{   
		n_regrow = diff / seconds_per_regrow_;
	}
	else
	{
		n_regrow = units_per_area_;
	}
	last_regen_ = now;
		//
		// a simple optimization.  If we'd regenerate everything, 
		// just delete all depletions.
		//
	if (n_regrow >= units_per_area_)
	{
		depletions_.clear();
	}
	else
	{
		Depletions::iterator itr = depletions_.begin(), end = depletions_.end();
		while (itr != end)
		{
			Depletions::iterator cur = itr;
			++itr;
			if (n_regrow >= (*cur).second)
				depletions_.erase( cur );
			else
				(*cur).second -= static_cast<unsigned short>(n_regrow);
		}
	}
}




ResourceDef::ResourceDef( const char* name ) :
	RegionGroup<ResourceRegion>( name ),
	initial_units_(0),
	current_units_(0)
{
}

void ResourceDef::read_config( ConfigElem& elem )
{
	initial_units_ = elem.remove_ulong( "InitialUnits" );
		
		// start current_units at initial_units in case there's no entry in 
		// RESOURCE.DAT
	current_units_ = initial_units_;
	
	unsigned short tmp;
	while (elem.remove_prop( "LandTile", &tmp ))
	{
		landtiles_.insert( tmp );
	}
	while (elem.remove_prop( "ObjType", &tmp ))
	{
		tiles_.insert( tmp );
	}
}

void ResourceDef::read_data( ConfigElem& elem )
{
	current_units_ = static_cast<int>(elem.remove_ulong( "Units" ));
}

bool ResourceDef::findmarker( xcoord x, ycoord y, Realm* realm, unsigned int objtype )
{
	if (!landtiles_.count(getgraphic(objtype)) && !tiles_.count(getgraphic(objtype)))
		return false;
	if (realm->findstatic(static_cast<unsigned short>(x),
							   static_cast<unsigned short>(y),
							   static_cast<unsigned short>(objtype)))
	{
		return true;
	}
	
 // FIXME range can be bad
	MAPTILE_CELL cell = realm->getmaptile( static_cast<unsigned short>(x), static_cast<unsigned short>(y) );
	return (cell.landtile == objtype); // FIXME blech! objtype == landtile? eh? well broken anyway.
}

#include "udatfile.h"
void ResourceDef::counttiles()
{
	unsigned int tilecount = 0;

	std::vector<Realm*>::iterator itr;
	for( itr = Realms->begin(); itr != Realms->end(); ++itr )
	{
		for( unsigned short x = 0; x < (*itr)->width(); ++x )
		{
			for( unsigned short y = 0; y < (*itr)->height(); ++y )
			{
				MAPTILE_CELL cell = (*itr)->getmaptile( x, y );

				if (landtiles_.count( cell.landtile ))
				{
					++tilecount;
					ResourceRegion* rgn = getregion( x, y, *itr );
					if (rgn) ++rgn->tilecount_;
				}

			}
		}
	}
	cout << "Resource " << name() << ": " << tilecount << endl;
	Log( "Resource %s: %ld\n", name().c_str(), tilecount );
	for( unsigned i = 0; i < regions_.size(); ++i )
	{
		ResourceRegion* rrgn = static_cast<ResourceRegion*>(regions_[i]);
		cout << "Region " << regions_[i]->name() << ": " << rrgn->tilecount_ << endl;
		Log( "Region %s: %ld\n", regions_[i]->name().c_str(), rrgn->tilecount_ );
	}
}

void ResourceDef::regenerate( time_t now )
{
	for( unsigned i = 0; i < regions_.size(); ++i )
	{
		ResourceRegion* rrgn = static_cast<ResourceRegion*>(regions_[i]);
		rrgn->regenerate( now );
	}
}

void ResourceDef::consume( unsigned amount )
{
	current_units_ -= amount;
}

void ResourceDef::produce( unsigned amount )
{
	current_units_ += amount;
}

typedef std::map<std::string,ResourceDef*> ResourceDefs;
ResourceDefs resourcedefs;

void regen_resources()
{
	THREAD_CHECKPOINT( tasks, 700 );
	time_t now = poltime();
	for( ResourceDefs::iterator itr = resourcedefs.begin(), end = resourcedefs.end(); itr != end; ++itr )
	{
		(*itr).second->regenerate( now );
	}
	THREAD_CHECKPOINT( tasks, 799 );
}
void count_resource_tiles()
{
	for( ResourceDefs::iterator itr = resourcedefs.begin(), end = resourcedefs.end(); itr != end; ++itr )
	{
		(*itr).second->counttiles();
	}
}
// regenerate resources every 10 minutes
PeriodicTask regen_resources_task( regen_resources, 10, 60*10, "RsrcRegen" );

BObjectImp* get_harvest_difficulty( const char* resource,
									xcoord x, 
									ycoord y, 
									Realm* realm,
									unsigned short marker )
{
	ResourceDefs::iterator itr = resourcedefs.find( resource );
	if (itr == resourcedefs.end())
		return new BError( "No resource by that name" );
	
	ResourceDef* rd = (*itr).second;
	if (!rd->findmarker(x,y,realm,marker))
		return new BError( "No resource-bearing landmark there" );

	ResourceRegion* rgn = rd->getregion( x, y, realm );
	if (rgn == NULL)
		return new BError( "No resource region at that location" );

	return rgn->get_harvest_difficulty(x,y, realm);
}

BObjectImp* harvest_resource( const char* resource,
							  xcoord x, ycoord y, Realm* realm,
							  int b, int n )
{
	ResourceDefs::iterator itr = resourcedefs.find( resource );
	if (itr == resourcedefs.end())
		return new BError( "No resource by that name" );
	
	ResourceDef* rd = (*itr).second;

/* assume GetHarvestDifficulty was already checked
	if (!rd->findmarker(x,y,marker))
		return new BError( "No resource-bearing landmark there" );
*/
	ResourceRegion* rgn = rd->getregion( x, y, realm );
	if (rgn == NULL)
		return new BError( "No resource region at that location" );

	return rgn->harvest_resource( x,y,b,n );
}

BObjectImp* get_region_string( const char* resource, 
							   xcoord x, ycoord y, Realm* realm, 
							   const string& propname )
{
	ResourceDefs::iterator itr = resourcedefs.find( resource );
	if (itr == resourcedefs.end())
		return new BError( "No resource by that name" );

	ResourceDef* rd =(*itr).second;

	ResourceRegion* rgn = rd->getregion( x, y, realm );
	if (rgn == NULL)
		return new BError( "No resource region at that location" );

	return rgn->get_region_string( propname );
}


ResourceDef* find_resource_def( const std::string& rname )
{
	ResourceDefs::iterator itr = resourcedefs.find( rname );
	if (itr == resourcedefs.end())
	{
		return NULL;
	}
	else
	{
		return (*itr).second;
	}
}


void read_resource_cfg( const char* resource )
{
	std::string filename = string("regions/") + resource + string( ".cfg" );
	ConfigFile cf( filename.c_str(), "GLOBAL REGION" );
	ConfigElem elem;
	
	auto_ptr<ResourceDef> rd( new ResourceDef( resource ) );

	while (cf.read( elem ))
	{
		if (elem.type_is( "global" ))
		{
			rd->read_config( elem );
		}
		else
		{
			rd->read_region( elem );
		}
	}

	resourcedefs[resource] = rd.release();
}

void load_resource_cfg()
{
	if ( !FileExists("regions/resource.cfg") )
	{
		if ( config.loglevel > 1 )
			cout << "File regions/resource.cfg not found, skipping.\n";
		return;
	}

	ConfigFile cf("regions/resource.cfg", "ResourceSystem");
	ConfigElem elem;
	while ( cf.read(elem) )
	{
		string resourcename;
		while ( elem.remove_prop("ResourceType", &resourcename) )
		{
			read_resource_cfg(resourcename.c_str());
		}
	}
	if ( config.count_resource_tiles )
		count_resource_tiles();
}




void read_global_data( ConfigElem& elem )
{
	ResourceDef* rd = find_resource_def( elem.rest() );
	if (rd == NULL)
	{
		cerr << "Error reading RESOURCE.DAT: Unable to find resource type " << elem.rest() << endl;
		throw runtime_error( "Data file error" );
	}

	rd->read_data( elem );
}
void read_region_data( ConfigElem& elem )
{
	ResourceDef* rd = find_resource_def( elem.rest() );
	if (rd == NULL)
	{
		cerr << "Error reading RESOURCE.DAT: Unable to find resource type " << elem.rest() << endl;
		throw runtime_error( "Data file error" );
	}
	std::string regionname = elem.remove_string( "Name" );
	ResourceRegion* rgn = rd->getregion( regionname );
	if (rgn == NULL)
	{
		cerr << "Error reading RESOURCE.DAT: Unable to find region " << regionname << " in resource " << elem.rest() << endl;
		throw runtime_error( "Data file error" );
	}
	rgn->read_data( elem );
}

void read_resources_dat()
{
	string resourcefile = config.world_data_path + "resource.dat";

	if (FileExists( resourcefile ))
	{
		ConfigFile cf( resourcefile, "GLOBALRESOURCEPOOL REGIONALRESOURCEPOOL" );
		ConfigElem elem;
		while (cf.read( elem ))
		{
			if (elem.type_is( "GlobalResourcePool" ))
			{
				read_global_data( elem );
			}
			else
			{
				read_region_data( elem );
			}
		}
	}
}

void ResourceDef::write( ostream& os ) const
{
	os << "GlobalResourcePool " << name() << endl
	   << "{" << endl
	   << "\tUnits\t" << current_units_ << endl
	   << "}" << endl
	   << endl;

	for( unsigned i = 0; i < regions_.size(); ++i )
	{
		ResourceRegion* rrgn = static_cast<ResourceRegion*>(regions_[i]);
		rrgn->write( os, name() );
	}
}

void ResourceRegion::write( ostream& os, const std::string& resource_name ) const
{
	os << "RegionalResourcePool " << resource_name << endl
	   << "{" << endl
	   << "\tName\t" << name_ << endl
	   << "\tUnits\t" << units_ << endl
	   << "#\t(regions/" << resource_name << ".cfg: Capacity is " << capacity_ << ")" << endl
	   << "}" << endl
	   << endl;
}

void write_resources_dat( ostream& ofs_resource )
{
   
	for( ResourceDefs::iterator itr = resourcedefs.begin(), end = resourcedefs.end(); itr != end; ++itr )
	{
		(*itr).second->write( ofs_resource );
	}

}

void clean_resources()
{
	for( ResourceDefs::iterator itr = resourcedefs.begin(), end = resourcedefs.end(); itr != end; ++itr )
	{
		delete (*itr).second;
	}
	resourcedefs.clear();
}

