/*
History
=======
2005/06/01 Shinigami: added getmapshapes - to get access to mapshapes
                      added getstatics - to fill a list with statics
2005/06/06 Shinigami: added readmultis derivative - to get a list of statics

Notes
=======

*/

#ifndef PLIB_REALM_H
#define PLIB_REALM_H

#include "realmdescriptor.h"
#include "staticblock.h"
#include "pol/uconst.h" // for MOVEMODE under gcc, bleh
#include "pol/udatfile.h"

class Character;
class Item;
class Items;
struct LosObj;
class MapServer;
class MapShapeList;
struct MAPTILE_CELL;
class MapTileServer;
enum MOVEMODE;
class StaticServer;
class UMulti;
class UObject;
struct Zone;

typedef vector<UMulti*> MultiList;

class Realm
{
public:
    explicit Realm( const string& realm_name, const string& realm_path="" );
	explicit Realm( const string& realm_name, Realm* realm );
    ~Realm();
	bool is_shadowrealm;
	unsigned int shadowid;
	Realm* baserealm;
	const string shadowname;

    unsigned width() const;
    unsigned height() const;

	unsigned season() const;

    bool valid( unsigned x, unsigned y, int z ) const;
	const string name() const;

    bool walkheight(unsigned x, unsigned y, int oldz, 
                    int* newz, 
                    UMulti** pmulti, Item** pwalkon,
                    bool doors_block,
                    MOVEMODE movemode,
					int* gradual_boost = NULL);
    bool walkheight( const Character* chr, unsigned x, unsigned y, int oldz, 
                     int* newz, 
                     UMulti** pmulti, Item** pwalkon,
					 int* gradual_boost = NULL);

	bool lowest_walkheight(	unsigned x, unsigned y, int oldz, 
							int* newz, 
							UMulti** pmulti, Item** pwalkon,
							bool doors_block,
							MOVEMODE movemode,
							int* gradual_boost = NULL);

    bool dropheight( unsigned dropx, 
                     unsigned dropy, 
                     int dropz,
                     int chrz, 
                     int* newz, 
                     UMulti** pmulti );

    bool has_los( const LosObj& att, const LosObj& tgt ) const;
    bool has_los( const UObject& iatt, const UObject& itgt ) const;
    bool has_los( const Character& iatt, const UObject& itgt ) const;

    bool navigable( unsigned x, unsigned y, int z, int height  ) const;

    UMulti* find_supporting_multi( unsigned x, unsigned y, int z );

    bool lowest_standheight( unsigned x, unsigned y, int* z ) const;
    bool findstatic( unsigned x, unsigned y, unsigned objtype ) const;
    void getstatics( StaticEntryList& statics, unsigned x, unsigned y ) const;
    bool groundheight( unsigned x, unsigned y, int* z ) const;
    MAPTILE_CELL getmaptile( unsigned x, unsigned y ) const;
    void getmapshapes( MapShapeList& shapes, unsigned x, unsigned y, unsigned long anyflags ) const;
    void readmultis( MapShapeList& vec, unsigned x, unsigned y, unsigned long flags ) const;
    void readmultis( MapShapeList& vec, unsigned x, unsigned y, unsigned long flags, MultiList& mvec );
    void readmultis( StaticList& vec, unsigned x, unsigned y ) const;

	Zone** zone;
	set<unsigned long> global_hulls; //xy-smashed together
    unsigned long toplevel_item_count;
    unsigned long mobile_count;
	unsigned getUOMapID() {return _Descriptor().uomapid;};
	unsigned getNumStaticPatches() {return _Descriptor().num_static_patches;};
	unsigned getNumMapPatches() {return _Descriptor().num_map_patches;};
	inline unsigned long encode_global_hull( unsigned short ax, unsigned short ay )
	{
		return (static_cast<unsigned long>(ax) << 16) | ay;
	}
protected:
    void standheight( MOVEMODE movemode, 
                      MapShapeList& shapes, 
                      int oldz, bool* result, 
                      int* newz,
					  int* gradual_boost = NULL ) const;

	 void lowest_standheight( MOVEMODE movemode, 
							  MapShapeList& shapes, 
							  int oldz, bool* result, 
							  int* newz,
							  int* gradual_boost = NULL ) const;

    bool dropheight( MapShapeList& shapes, 
                     int dropz,
                     int chrz, 
                     int* newz ) const;

    void readdynamics( MapShapeList& vec, 
                       unsigned x, 
                       unsigned y, 
                       Items& walkon_items, 
                       bool doors_block );

    bool dynamic_item_blocks_los( const LosObj& att, const LosObj& target, 
                              unsigned x, unsigned y, int z ) const;

    bool static_item_blocks_los( unsigned x, unsigned y, int z ) const;
    bool los_blocked( const LosObj& att, const LosObj& target, 
                  unsigned x, unsigned y, int z ) const;

    UMulti* find_supporting_multi( MultiList& mvec, int z );

private:
    RealmDescriptor _descriptor;
public:
    auto_ptr<MapServer> _mapserver;
    auto_ptr<StaticServer> _staticserver;
    auto_ptr<MapTileServer> _maptileserver;
private:
    // not implemented:
    Realm& operator=( const Realm& );
    Realm( const Realm& );

public:
	inline RealmDescriptor _Descriptor() const
	{
		if (is_shadowrealm)
			return baserealm->_Descriptor();
		return _descriptor;
	}
};

#endif
