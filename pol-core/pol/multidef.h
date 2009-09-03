/*
History
=======
2009/09/01 Turley:   VS2005/2008 Support moved inline MultiDef::getkey to .h

Notes
=======

*/

#ifndef MULTIDEF_H
#define MULTIDEF_H

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

// also consider: multimap<unsigned long, unsigned long>
// unsigned long is (x << 16) | y
// unsigned long is z << 16 | objtype
// hell, even multimap<unsigned short,unsigned long>
// unsigned short is (x<<8)|y
// (relative signed x/y, -128 to +127
// unsigned long is z << 16 | objtype

#include <stdio.h>

#include "udatfile.h"
#include "clib/rawtypes.h"
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

    int xbase;          // x[0] is really x[xbase]
    int xsize;
    int ybase;
    int ysize;
    
    typedef vector<const MULTI_ELEM*> HullList;
    HullList hull;
    HullList internal_hull;
    typedef set<unsigned short> HullList2;
    HullList2 hull2;
    HullList2 internal_hull2;
    
    typedef multimap<unsigned short, const MULTI_ELEM*> Components;
    typedef pair<Components::const_iterator,Components::const_iterator> ItrPair;

        int minrx, minry, minrz; // minimum relative distances
        int maxrx, maxry, maxrz;
        Components components;

        static int global_minrx;
        static int global_minry;
        static int global_minrz;
        static int global_maxrx;
        static int global_maxry;
        static int global_maxrz;
        
    ItrPair findcomponents( int rx, int ry);

    bool findcomponents( Components::const_iterator& beg, Components::const_iterator& end,
                         int rx, int ry) const;

    unsigned short getkey( int rx, int ry ) const;

        // returns true if it finds anything at this rx,ry
    bool readobjects( StaticList& vec, int rx, int ry, int zbase ) const;
    bool readshapes( MapShapeList& vec, int rx, int ry, int zbase, unsigned long anyflags ) const;

    bool body_contains( int rx, int ry ) const;
    const MULTI_ELEM* find_component( int rx, int ry ) const;

    void add_to_hull( const MULTI_ELEM* elem );
    void add_to_internal_hull( const MULTI_ELEM* elem );
    void add_row_tohull( int y );
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

inline unsigned short MultiDef::getkey( int rx, int ry ) const
{
    unsigned char crx = static_cast<unsigned char>(rx);
    unsigned char cry = static_cast<unsigned char>(ry);
    
    unsigned short key = (crx << 8) | cry;
    return key;
}

#endif
