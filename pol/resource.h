/*
History
=======


Notes
=======

*/

#ifndef RESOURCE_H
#define RESOURCE_H

#include <iosfwd>
#include <set>

#include "poltype.h"
#include "proplist.h"
#include "region.h"

class BObjectImp;
class ConfigElem;

void remove_resources( u16 objtype, u16 amount );
void return_resources( u16 objtype, u16 amount );

BObjectImp* get_harvest_difficulty( const char* resource,
                                    xcoord x, 
                                    ycoord y, Realm* realm,
                                    unsigned short marker );
BObjectImp* harvest_resource( const char* resource,
                              xcoord x, ycoord y, Realm* realm,
                              int b, int n );

BObjectImp* get_region_string( const char* resource, 
                               xcoord x, ycoord y, Realm* realm,
                               const string& propname );

ResourceDef* find_resource_def( const std::string& rname );
void write_resources_dat( std::ostream& ofs_resource );
void read_resources_dat();
void clean_resources();


class ResourceRegion : public Region
{
public:
    explicit ResourceRegion( ConfigElem& elem );

    void read_data( ConfigElem& elem );

    BObjectImp* get_harvest_difficulty( xcoord x, ycoord y, Realm* realm );
    BObjectImp* harvest_resource( xcoord x, ycoord y, int b, int n );

    void regenerate( time_t now );
    void write( std::ostream& os, const std::string& resource_name ) const;

    unsigned long tilecount_;
    friend class ResourceDef;
private:

        //
        // These are only used for depletion levels.  
        //
    const unsigned long units_per_area_;
    const unsigned long seconds_per_regrow_;
    typedef std::map<unsigned long, unsigned short> Depletions;
    Depletions depletions_;
    time_t last_regen_;
    

        //
        // the following are used if resource banks are implemented.
        //
    unsigned long capacity_;        // config
    unsigned long units_;           // data
};


class ResourceDef : public RegionGroup<ResourceRegion>
{
public:
    explicit ResourceDef( const char* name );
    void read_config( ConfigElem& elem );
    void read_data( ConfigElem& elem );
    // void read_region( ConfigElem& elem );

    bool findmarker( xcoord x, ycoord y, Realm* realm, unsigned short objtype );
    void regenerate( time_t now );
    void consume( unsigned amount );
    void produce( unsigned amount );
    void counttiles();
    void write( std::ostream& os ) const;
protected:

private:
    unsigned long initial_units_;
    long current_units_;
    std::set<unsigned short> landtiles_;
    std::set<unsigned short> tiles_;
    // std::vector<ResourceRegion*> regions_;
};

#endif // RESOURCE_H
