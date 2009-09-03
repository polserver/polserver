/*
History
=======


Notes
=======

*/

#ifndef MISCRGN_H
#define MISCRGN_H

#include "plib/realm.h"

#include "region.h"


class NoCastRegion : public Region
{
    typedef Region base;
public:
    NoCastRegion( ConfigElem& elem );

    bool nocast() const;

private:
    bool nocast_;
};

inline bool NoCastRegion::nocast() const
{
    return nocast_;
}

typedef RegionGroup<NoCastRegion> NoCastDef;

extern NoCastDef* nocastdef;



class LightRegion : public Region
{
    typedef Region base;
public:
    LightRegion( ConfigElem& elem );
    unsigned lightlevel;
};

typedef RegionGroup<LightRegion> LightDef;
extern LightDef* lightdef;



class WeatherRegion : public Region
{
    typedef Region base;
public:
    WeatherRegion( ConfigElem& elem );
    void setweather( unsigned char weathertype,
                     unsigned char severity,
                     unsigned char aux );
    unsigned char weathertype;
    unsigned char severity;
    unsigned char aux;
    int lightoverride; // if -1, no override
};

class WeatherDef : public RegionGroup<WeatherRegion>
{
public:
    WeatherDef( const char* name );
	virtual ~WeatherDef();
    void copy_default_regions();

    bool assign_zones_to_region( const char* regionname,
                                 unsigned short x1, unsigned short y1,
                                 unsigned short x2, unsigned short y2,
								 Realm* realm );
private:
	RegionRealms default_regionrealms;
    //RegionId default_regionidx_[ZONE_X][ZONE_Y]; // 1-based
};
extern WeatherDef* weatherdef;


#endif
