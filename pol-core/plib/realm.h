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
#include "../pol/uconst.h" // for MOVEMODE under gcc, bleh
#include "../pol/udatfile.h"

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

typedef std::vector<UMulti*> MultiList;

class Realm
{
public:
    explicit Realm( const std::string& realm_name, const std::string& realm_path="" );
	explicit Realm( const std::string& realm_name, Realm* realm );
    ~Realm();
	bool is_shadowrealm;
	unsigned int shadowid;
	Realm* baserealm;
	const std::string shadowname;

    unsigned short width() const;
    unsigned short height() const;

	unsigned season() const;

    bool valid( unsigned short x, unsigned short y, short z ) const;
	const std::string name() const;

    bool walkheight(unsigned short x, unsigned short y, short oldz, 
                    short* newz, 
                    UMulti** pmulti, Item** pwalkon,
                    bool doors_block,
                    MOVEMODE movemode,
					short* gradual_boost = NULL);
    bool walkheight( const Character* chr, unsigned short x, unsigned short y, short oldz, 
                     short* newz, 
                     UMulti** pmulti, Item** pwalkon,
					 short* gradual_boost = NULL);

	bool lowest_walkheight(	unsigned short x, unsigned short y, short oldz, 
							short* newz, 
							UMulti** pmulti, Item** pwalkon,
							bool doors_block,
							MOVEMODE movemode,
							short* gradual_boost = NULL);

    bool dropheight( unsigned short dropx, 
                     unsigned short dropy, 
                     short dropz,
                     short chrz, 
                     short* newz, 
                     UMulti** pmulti );

    bool has_los( const LosObj& att, const LosObj& tgt ) const;
    bool has_los( const UObject& iatt, const UObject& itgt ) const;
    bool has_los( const Character& iatt, const UObject& itgt ) const;

    bool navigable( unsigned short x, unsigned short y, short z, short height  ) const;

    UMulti* find_supporting_multi( unsigned short x, unsigned short y, short z );

    bool lowest_standheight( unsigned short x, unsigned short y, short* z ) const;
    bool findstatic( unsigned short x, unsigned short y, unsigned short objtype ) const;
    void getstatics( StaticEntryList& statics, unsigned short x, unsigned short y ) const;
    bool groundheight( unsigned short x, unsigned short y, short* z ) const;
    MAPTILE_CELL getmaptile( unsigned short x, unsigned short y ) const;
    void getmapshapes( MapShapeList& shapes, unsigned short x, unsigned short y, unsigned int anyflags ) const;
    void readmultis( MapShapeList& vec, unsigned short x, unsigned short y, unsigned int flags ) const;
    void readmultis( MapShapeList& vec, unsigned short x, unsigned short y, unsigned int flags, MultiList& mvec );
    void readmultis( StaticList& vec, unsigned short x, unsigned short y ) const;

	Zone** zone;
	set<unsigned int> global_hulls; //xy-smashed together
    unsigned int toplevel_item_count;
    unsigned int mobile_count;
	unsigned getUOMapID() {return _Descriptor().uomapid;};
	unsigned getNumStaticPatches() {return _Descriptor().num_static_patches;};
	unsigned getNumMapPatches() {return _Descriptor().num_map_patches;};
	inline unsigned int encode_global_hull( unsigned short ax, unsigned short ay )
	{
		return (static_cast<unsigned int>(ax) << 16) | ay;
	}
protected:
    void standheight( MOVEMODE movemode, 
                      MapShapeList& shapes, 
                      short oldz, bool* result, 
                      short* newz,
					  short* gradual_boost = NULL ) const;

	 void lowest_standheight( MOVEMODE movemode, 
							  MapShapeList& shapes, 
							  short oldz, bool* result, 
							  short* newz,
							  short* gradual_boost = NULL ) const;

    bool dropheight( MapShapeList& shapes, 
                     short dropz,
                     short chrz, 
                     short* newz ) const;

    void readdynamics( MapShapeList& vec, 
                       unsigned short x, 
                       unsigned short y, 
                       Items& walkon_items, 
                       bool doors_block );

    bool dynamic_item_blocks_los( const LosObj& att, const LosObj& target, 
                              unsigned short x, unsigned short y, short z ) const;

    bool static_item_blocks_los( unsigned short x, unsigned short y, short z ) const;
    bool los_blocked( const LosObj& att, const LosObj& target, 
                  unsigned short x, unsigned short y, short z ) const;

    UMulti* find_supporting_multi( MultiList& mvec, short z );

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
