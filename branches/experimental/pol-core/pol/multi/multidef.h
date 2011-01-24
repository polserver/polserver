/*
History
=======
2009/09/01 Turley:    VS2005/2008 Support moved inline MultiDef::getkey to .h
2009/09/03 MuadDib:   Relocation of multi related cpp/h

Notes
=======

*/

#ifndef MULTIDEF_H
#define MULTIDEF_H

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

// also consider: multimap<unsigned int, unsigned int>
// unsigned int is (x << 16) | y
// unsigned int is z << 16 | objtype
// hell, even multimap<unsigned short,unsigned int>
// unsigned short is (x<<8)|y
// (relative signed x/y, -128 to +127
// unsigned int is z << 16 | objtype

#include <stdio.h>

#include "../udatfile.h"
#include "../../clib/rawtypes.h"

class ConfigElem;
class MapShapeList;

struct USTRUCT_MULTI_ELEMENT;

struct MULTI_ELEM
{
    unsigned short objtype;
    s16 x;
    s16 y;
    s16 z;
    bool is_static;
};

class MultiDef
{
public:
    explicit MultiDef( ConfigElem& elem, u16 multiid );
    ~MultiDef();

    u16 multiid;
    enum { UNKNOWN, BOAT, HOUSE, STAIRS } type;
    u16 graphic;

    vector< MULTI_ELEM > elems;

    short xbase;          // x[0] is really x[xbase]
    short xsize;
    short ybase;
    short ysize;
    
    typedef vector<const MULTI_ELEM*> HullList;
    HullList hull;
    HullList internal_hull;
    typedef set<unsigned short> HullList2;
    HullList2 hull2;
    HullList2 internal_hull2;
    
    typedef multimap<unsigned short, const MULTI_ELEM*> Components;
    typedef pair<Components::const_iterator,Components::const_iterator> ItrPair;

        short minrx, minry, minrz; // minimum relative distances
        short maxrx, maxry, maxrz;
        Components components;

        static short global_minrx;
        static short global_minry;
        static short global_minrz;
        static short global_maxrx;
        static short global_maxry;
        static short global_maxrz;
        
    ItrPair findcomponents( short rx, short ry);

    bool findcomponents( Components::const_iterator& beg, Components::const_iterator& end,
                         short rx, short ry) const;

    unsigned short getkey( short rx, short ry ) const;

        // returns true if it finds anything at this rx,ry
    bool readobjects( StaticList& vec, short rx, short ry, short zbase ) const;
    bool readshapes( MapShapeList& vec, short rx, short ry, short zbase, unsigned int anyflags ) const;

    bool body_contains( short rx, short ry ) const;
    const MULTI_ELEM* find_component( short rx, short ry ) const;

    void add_to_hull( const MULTI_ELEM* elem );
    void add_to_internal_hull( const MULTI_ELEM* elem );
    void add_row_tohull( short y );
    void add_body_tohull();
    void eliminate_hull_dupes();
    void computehull();
    void addrec( const MULTI_ELEM* elem );
    void fill_hull2();

    void init();
};

typedef map< u16, MultiDef* > MultiDefs;

bool MultiDefByGraphicExists( u16 graphic );
const MultiDef* MultiDefByGraphic( u16 graphic );
//extern MultiDefs multishapes_by_graphic;

extern MultiDefs multidefs_by_multiid;

typedef map<u16,u16> ObjtypeByMultiID;
extern ObjtypeByMultiID objtype_by_multiid;

void clean_multidefs();

inline unsigned short MultiDef::getkey( short rx, short ry ) const
{
    unsigned char crx = static_cast<unsigned char>(rx);
    unsigned char cry = static_cast<unsigned char>(ry);
    
    unsigned short key = (crx << 8) | cry;
    return key;
}

#endif
