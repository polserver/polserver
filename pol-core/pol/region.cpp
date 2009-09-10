/*
History
=======
2005/01/23 Shinigami: RegionGroupBase::Con-/Destructor - Tokuno MapDimension doesn't fit blocks of 32x32 (ZONE_SIZE)

Notes
=======

*/

#include "clib/stl_inc.h"
#include "bscript/berror.h"
#include "bscript/impstr.h"

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"
#include "clib/stlutil.h"

#include "realms.h"
#include "region.h"
#include "uworld.h"

Region::Region( ConfigElem& elem ) :
    name_(elem.rest())
{
}

Region::~Region()
{
}

void Region::read_custom_config( ConfigElem& elem )
{
    proplist_.readRemainingPropertiesAsStrings( elem );
}

BObjectImp* Region::get_region_string( const string& propname )
{
    string propvalue;
    if (proplist_.getprop( propname, propvalue ))
    {
        return new String( propvalue );
    }
    else
    {
        return new BError( "Property not found" );
    }
}



RegionGroupBase::RegionGroupBase( const char* name ) :
    name_(name)
{
    //memset( &regionidx_, 0, sizeof regionidx_ );
	std::vector<Realm*>::const_iterator itr;
	for( itr = Realms->begin(); itr != Realms->end(); ++itr )
	{
		unsigned int gridwidth = (*itr)->width() / ZONE_SIZE;
		unsigned int gridheight = (*itr)->height() / ZONE_SIZE;

    // Tokuno-Fix removed Turley, 2009/09/08 (for ZONE_SIZE 4 not needed)
    /*if (gridwidth * ZONE_SIZE < (*itr)->width())
      gridwidth++;
    if (gridheight * ZONE_SIZE < (*itr)->height())
      gridheight++;*/

		RegionId** zone = new RegionId*[gridwidth];

		for(unsigned int i=0; i<gridwidth; i++)
		{
			zone[i] = new RegionId[gridheight];
			for(unsigned int j=0; j<gridheight; j++)
			{
				zone[i][j] = 0;
			}
		}
		regionrealms.insert( make_pair(*itr,zone) );
	}
}
RegionGroupBase::~RegionGroupBase()
{
	RegionRealms::iterator itr;
	for( itr = regionrealms.begin(); itr != regionrealms.end(); ++itr )
	{
		unsigned int gridwidth = itr->first->width() / ZONE_SIZE;

    // Tokuno-Fix removed Turley, 2009/09/08 (for ZONE_SIZE 4 not needed)
    /*if (gridwidth * ZONE_SIZE < itr->first->width())
      gridwidth++;*/

    for(unsigned int i=0; i<gridwidth; i++)
			delete[] itr->second[i];
		delete[] itr->second;
	}

	// cleans the regions_ vector...
	std::vector<Region*>::iterator reg_itr = regions_.begin(), reg_end = regions_.end();
	for ( ; reg_itr != reg_end; reg_itr++) {
			delete *reg_itr;
	}
	regions_.clear();

}

void RegionGroupBase::paint_zones( ConfigElem& elem, RegionId ridx )
{
    string zonestr, strrealm;

	strrealm = elem.remove_string("Realm", "britannia");
	Realm* realm = find_realm(strrealm);
	if(!realm)
		elem.throw_error( "Realm not found" );
    while (elem.remove_prop( "Range", &zonestr ))
    {
        unsigned short xwest, ynorth, xeast, ysouth;
        ISTRINGSTREAM is(zonestr);
        if (is >> xwest >> ynorth >> xeast >> ysouth)
        {
            if (xeast >= realm->width() || ysouth >= realm->height())
            {
                elem.throw_error( "Zone range is out of bounds for realm" );
            }
            unsigned zone_xwest, zone_ynorth, zone_xeast, zone_ysouth;
            XyToZone( xwest, ynorth, &zone_xwest, &zone_ynorth );
            XyToZone( xeast, ysouth, &zone_xeast, &zone_ysouth );
            unsigned zx, zy;
            for( zx = zone_xwest; zx <= zone_xeast; ++zx )
            {
                for( zy = zone_ynorth; zy <= zone_ysouth; ++zy )
                {
                    /*
                    if (zones[zx][zy].regions[regiontype])
                    {
                        elem.throw_error( "Zone is already allocated" );
                    }
                    */
                    regionrealms[realm][zx][zy] = ridx;
                }
            }
        }
        else
        {
            elem.throw_error( "Poorly formed zone range: " + zonestr );
        }
    }
}

RegionId RegionGroupBase::getregionid( xcoord x, ycoord y, Realm* realm )
{
    unsigned zx, zy;
    XyToZone(x,y,&zx,&zy);
	RegionId** regiongrp;
	if (realm->is_shadowrealm)
		regiongrp = regionrealms[realm->baserealm];
	else
		regiongrp = regionrealms[realm];
    return regiongrp[zx][zy];
}

Region* RegionGroupBase::getregion_byname( const std::string& regionname )
{
    RegionsByName::iterator itr = regions_byname_.find( regionname );
    if (itr == regions_byname_.end())
        return NULL;
    else
        return (*itr).second;
}

Region* RegionGroupBase::getregion_byloc( xcoord x, ycoord y, Realm* realm )
{
    RegionId ridx = getregionid( x, y, realm );

	//dave 12-22 return null if no regions, don't throw
	std::vector<Region*>::iterator itr = regions_.begin();
	if((itr += ridx) >= regions_.end())
		return NULL;
	else
		return regions_[ ridx ];
}

void RegionGroupBase::read_region( ConfigElem& elem )
{
    Region* rgn = create_region( elem );

    regions_.push_back( rgn );
    regions_byname_.insert( RegionsByName::value_type(elem.rest(), rgn) );

    paint_zones( elem, static_cast<RegionId>(regions_.size()-1) );
    rgn->read_custom_config( elem );
}

void RegionGroupBase::create_bgnd_region( ConfigElem& elem )
{
    Region* rgn = create_region( elem );

    regions_.push_back( rgn );
    regions_byname_.insert( RegionsByName::value_type("_background_", rgn) );
    rgn->read_custom_config( elem );
}

void read_region_data( RegionGroupBase& grp,
                       const char* preferred_filename,
                       const char* other_filename,
                       const char* tags_expected )
{
    const char* filename;
    if (FileExists( preferred_filename ))
        filename = preferred_filename;
    else
        filename = other_filename;

    if (FileExists( filename ))
    {
        ConfigFile cf( filename, tags_expected );
        ConfigElem elem;

        grp.create_bgnd_region( elem );

        while (cf.read( elem ))
        {
            grp.read_region( elem );
        }
    }
}
