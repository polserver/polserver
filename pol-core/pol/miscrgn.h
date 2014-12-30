/*
History
=======


Notes
=======

*/

#ifndef MISCRGN_H
#define MISCRGN_H

#include "../plib/realm.h"

#include "region.h"

namespace Pol {
  namespace Core {
	class NoCastRegion : public Region
	{
	  typedef Region base;
	public:
	  NoCastRegion( Clib::ConfigElem& elem, RegionId id );

	  bool nocast() const;

	private:
	  bool nocast_;
	};

	inline bool NoCastRegion::nocast() const
	{
	  return nocast_;
	}


	class LightRegion : public Region
	{
	  typedef Region base;
	public:
      LightRegion( Clib::ConfigElem& elem, RegionId id );
	  unsigned lightlevel;
	};


	class WeatherRegion : public Region
	{
	  typedef Region base;
	public:
      WeatherRegion( Clib::ConfigElem& elem, RegionId id );
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
								   Plib::Realm* realm );
	private:
	  RegionRealms default_regionrealms;
	  //RegionId default_regionidx_[ZONE_X][ZONE_Y]; // 1-based
	};
	
  }
}

#endif
