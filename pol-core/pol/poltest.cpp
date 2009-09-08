/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"


#include <assert.h>

#include "clib/passert.h"

#include "plib/maptile.h"
#include "plib/realm.h"

#include "realms.h"

#include "item/item.h"
#include "los.h"
#include "npc.h"
#include "uofile.h"
#include "polcfg.h"
#include "udatfile.h"
#include "uvars.h"
#include "skilladv.h"
#include "plib/testenv.h"

#include "poltest.h"

extern bool static_debug_on;

extern void create_test_environment();
extern void read_multidefs();

extern NPC* test_banker;
extern Item* test_guard_door;
extern NPC* test_water_elemental;
extern NPC* test_seaserpent;

#if ENABLE_OLD_MAPCODE
void test_lowest_standheight( int x, int y, int z )
{
    int actualz;
    cout << "LowestStandHeight(" << x << "," << y <<"): Expect " << z << ": ";
    uo_lowest_standheight( x, y, &actualz );
    
    cout << "Got " << actualz << ": ";

    if (actualz == z)
    {
        cout << "Success!" << endl;
        inc_successes();
    }
    else
    {
        cout << "Failure!" << endl;
        inc_failures();
    }
}
#endif
#if ENABLE_OLD_MAPCODE
void test_walk( Character* chr,
                int oldx, int oldy, int oldz, 
                int newx, int newy, bool exp_result, int exp_z )
{
    int newz;
    
    cout << "WalkHeight(" << newx << "," << newy << "," << oldz << "): " 
         << "Expect " << exp_result << "," << exp_z << ": ";
    UMulti* multi;
    Item* itm;
    bool res = uo_walkheight( chr, newx, newy,oldz, &newz, &multi, &itm );
    
    cout << "Got " << res << "," << newz << ": ";

    if (exp_result != res)
    {
        cout << "Failure!" << endl;
        inc_failures();
        return;
    }
    if (exp_result)     // Z is only defined if result is true
    {
        if (exp_z != newz)
        {
            cout << "Failure!" << endl;
            inc_failures();
            return;
        }
    }
    inc_successes();
    cout << "Ok!" << endl;
}
void test_walk( int oldx, int oldy, int oldz, 
                int newx, int newy, bool exp_result, int exp_z )
{
    int newz;
    
    cout << "WalkHeight(" << newx << "," << newy << "," << oldz << "): " 
         << "Expect " << exp_result << "," << exp_z << ": ";
    UMulti* multi;
    Item* itm;
    bool res = uo_walkheight( newx, newy,oldz, &newz, &multi, &itm );
    
    cout << "Got " << res << "," << newz << ": ";

    if (exp_result != res)
    {
        cout << "Failure!" << endl;
        inc_failures();
        return;
    }
    if (exp_result)     // Z is only defined if result is true
    {
        if (exp_z != newz)
        {
            cout << "Failure!" << endl;
            inc_failures();
            return;
        }
    }
    inc_successes();
    cout << "Ok!" << endl;
}


void walk_test_2()
{
	cout << "UO datafile mobile walk tests:" << endl;
    test_walk( test_water_elemental, 1482, 1780, -2,  1482, 1781, true, -5 );
    test_walk( test_water_elemental, 1482, 1850, -2,  1482, 1851, true, -5 );
    test_walk( test_water_elemental, 1474, 1766, -5,  1474, 1767, true, -5 );

    test_walk( test_seaserpent,      1479, 1763, -5,  1480, 1763, false, 0 );

    test_lowest_standheight( 1175, 2635, 0 );
}

/* This code should mirror code in drop_item_on_ground */
void test_drop( int chrx, int chry, int chrz, 
                int dropx, int dropy, int dropz, bool exp_result, int exp_z )
{
    int newz;
    UMulti* multi;
    bool result = uo_dropheight( dropx,dropy,chrz, &newz, &multi );
    if (exp_result != result)
    {
        cout << "Failure!" << endl;
        inc_failures();
        return;
    }
    if (result)
    {
        if (newz != exp_z)
        {
            cout << "Failure!" << endl;
            inc_failures();
            return;
        }
    }
    inc_successes();
    cout << "Ok!" << endl;
}

void drop_test( void )
{
    test_drop( 1328,1645,72,    1328,1644,78,  true,78 );
}
#endif

void map_test()
{
    MAPTILE_CELL cell = main_realm->getmaptile( 1453, 1794 );
}

#if ENABLE_OLD_MAPCODE
void statics_test()
{
    StaticList vec;
    readstatics( vec, 1452, 1794 );
}
#endif

void test_skilladv( unsigned long raw, unsigned short base )
{
    cout << "Raw " << raw << ", base " << base << ": ";
    if (raw_to_base( raw ) != base)
    {
        cout << "raw_to_base failed!" << endl;
        inc_failures();
    }
    if (base_to_raw( base ) != raw)
    {
        cout << "base_to_raw failed!" << endl;
        inc_failures();
    }
    cout << "Passed" << endl;
    inc_successes();
}

void skilladv_test()
{
	cout << "Skill advancement tests:" << endl;
    test_skilladv( 20, 1);
    test_skilladv( 512,     25 );
    test_skilladv( 1024,    50 );
    test_skilladv( 2048,    100 );
    test_skilladv( 3072,    150 );
    test_skilladv( 4096,    200 );
    test_skilladv( 5120,    225 );
    test_skilladv( 6144,    250 );
    test_skilladv( 7168,    275 );
    test_skilladv( 8192,    300 );
    test_skilladv( 16384,   400 );
    test_skilladv( 32768,   500 );
    test_skilladv( 65536,   600 );
    test_skilladv( 131072,  700 );
    test_skilladv( 262144,  800 );
    test_skilladv( 524288,  900 );
    test_skilladv( 1048576, 1000 );
    test_skilladv( 2097152, 1100 );
    test_skilladv( 4194304, 1200 );
    test_skilladv( 8388608, 1300 );
    test_skilladv( 16777216, 1400 );
    test_skilladv( 33554432, 1500 );
    test_skilladv( 67108864, 1600 );
    test_skilladv( 134217728, 1700 );
    test_skilladv( 268435456, 1800 );
    test_skilladv( 536870912, 1900 );

    for( unsigned short base = 0; base < 2000; ++base )
    {
        cout << "Base " << base << ": Raw=";
        unsigned long raw = base_to_raw( base );
        cout << raw << ": ";
        unsigned short calcbase = raw_to_base(raw);
        if (calcbase != base)
        {
            cout << "Failed (" << calcbase << ")" << endl;
            inc_failures();
        }
        else
        {
            cout << "Passed" << endl;
            inc_successes();
        }
    }

    for( unsigned long raw = 0; raw < 2500; ++raw )
    {
        cout << "Raw " << raw << ": Base=";
        unsigned short base = raw_to_base( raw );
        cout << base << ": ";
        unsigned long calcraw = base_to_raw(base);
        if (calcraw != raw)
        {
            cout << "Failed (" << calcraw << ")" << endl;
            inc_failures();
        }
        else
        {
            cout << "Passed" << endl;
            inc_successes();
        }
    }
}

#if ENABLE_OLD_MAPCODE
void test_los( UObject* src, UObject* target, bool should_have_los )
{
    cout << "LOS test: " << src->name() << " to " 
         << target->name() << "(" << should_have_los << "):";
    bool res = uo_has_los( *src, *target );
    cout << res << " ";
    if (should_have_los == res)
    {
        cout << "Ok!" << endl;
        inc_successes();
    }
    else
    {
        cout << "Failure!" << endl;
        inc_failures();
    }

}
void test_los( u16 x1, u16 y1, s8 z1,
               u16 x2, u16 y2, s8 z2,
               bool should_have_los )
{
    cout << "LOS test: (" << x1 << "," << y1 << "," << int(z1) << ")"
         << " - (" << x2 << "," << y2 << "," << int(z2) << ")"
         << " (" << should_have_los << "):";
    bool res = uo_has_los( LosObj(x1,y1,z1), LosObj(x2,y2,z2) );
    cout << res << " ";
    if (should_have_los == res)
    {
        cout << "Ok!" << endl;
        inc_successes();
    }
    else
    {
        cout << "Failure!" << endl;
        inc_failures();
    }

}
void los_test()
{
	cout << "UO datafile LOS tests:" << endl;
    test_los( test_banker, test_guard_door, true );
    test_los( 5421,78,10,    5422,89,20,  false );
    test_los( 1403,1624,28,  1402,1625,28, true );

    test_los( 1374,1625,59,  1379,1625,30, true );
    test_los( 1373,1625,59,  1379,1625,30, true );
    test_los( 1372,1625,59,  1379,1625,30, false );
    test_los( 1372,1625,59,  1381,1625,30, true );

    // from you standing in the house in GA to a backpack outside
    test_los( 5987, 1999, 7+9, 5991, 2001, 1, false );
    test_los( 5987, 1999, 7+9, 5993, 1997, 1, false );

    // test looking through "black space" in a cave
    test_los( 5583, 226, 0,  5590, 223, 0,  false );

    test_los( test_banker2, test_chest1, true );
    test_los( test_banker2, test_chest2, true );
}
void walk_test()
{
	cout << "UO datafile walk tests:" << endl;
    test_walk( 1344, 1637, 72,      1345, 1637,     true, 72 );

        // walk down some stairs in Castle Britannia
    test_walk( 1352, 1635, 72,      1352, 1634,     true, 72 );
    test_walk( 1352, 1634, 72,      1352, 1633,     true, 72 );
    test_walk( 1352, 1633, 72,      1352, 1632,     true, 69 );
    test_walk( 1352, 1632, 69,      1352, 1631,     true, 64 );
    test_walk( 1352, 1631, 64,      1352, 1630,     true, 59 );
    test_walk( 1352, 1630, 59,      1352, 1629,     true, 54 );
    test_walk( 1352, 1629, 54,      1352, 1628,     true, 50 );
    test_walk( 1352, 1628, 50,      1352, 1627,     true, 50 );

    test_walk( 1381, 1625, 30,      1382, 1625,     true, 30 );
    test_walk( 1382, 1626, 30,      1383, 1625,     true, 30 );

    test_walk( 1386, 1628, 30,      1386, 1629,     true, 30 );

    // on the bridge
    test_walk( 1402, 1626, 28,      1402, 1625,     true, 28 );
    // leaving the bridge
    test_walk( 1408, 1625, 28,      1409, 1625,     true, 28 );
    test_walk( 1409, 1625, 28,      1410, 1625,     true, 28 );
    test_walk( 1410, 1625, 28,      1411, 1625,     true, 26 );
    test_walk( 1411, 1625, 26,      1412, 1625,     true, 25 );
    test_walk( 1412, 1625, 25,      1413, 1625,     true, 24 );
    test_walk( 1413, 1625, 25,      1414, 1625,     true, 23 );
    test_walk( 1414, 1625, 25,      1415, 1625,     true, 22 );
    test_walk( 1415, 1625, 25,      1416, 1625,     true, 21 );
    test_walk( 1416, 1625, 25,      1417, 1625,     true, 20 );
    test_walk( 1417, 1625, 25,      1418, 1625,     true, 20 );
    test_walk( 1418, 1625, 25,      1419, 1625,     true, 20 );

    //static_debug_on = true;
    test_walk( 1377, 1759, 1,       1378, 1759,     true, 1 );

    // try walking through a static wall
    test_walk( 1345, 1649, 72,      1346, 1649,     false, 0 );

    // test the walking on a floor, with a dynamic below you, screwing up.
    test_walk( 1347, 1659, 72,      1347, 1658,     true, 72 );
    test_walk( 1347, 1658, 72,      1347, 1657,     true, 72 );


    // back on the bridge
    //static_debug_on = true;
    test_walk( 1398, 1624, 29,      1399, 1624,     true, 28 );

    // try walking through a portcullis
    test_walk( 1399, 1625, 27,      1398, 1625,     false, 0 );

    // try walking onto a stone step
    test_walk( 1310, 1630, 30,      1309, 1630,     true, 31 );

    // try walking onto a dungeon ramp
    test_walk( 1311, 1637, 30,      1311, 1636,     true, 32 );

    // try walking under a stone arch
    test_walk( 1311, 1650, 30,      1311, 1649,     true, 30 );

    // try walking through a secret door
    test_walk( 1316, 1649, 30,      1316, 1648,     true, 30 );

    // try walking through a hide-type door
    test_walk( 1314, 1654, 30,      1313, 1654,     true, 30 );

    // try walking under a dungeon arch
    test_walk( 1313, 1658, 30,      1313, 1657,     true, 30 );

    // try walking over a black pearl
    test_walk( 1315, 1657, 30,      1315, 1658,     true, 30 );

    // try walking around on a boat
    test_walk( 1487, 1770, -2,      1487, 1769,     true, -2 );
    test_walk( 1487, 1769, -2,      1487, 1768,     false, 0 ); // try to walk into the mast
    test_walk( 1487, 1769, -2,      1486, 1769,     true, -2 );
    test_walk( 1486, 1769, -2,      1486, 1768,     true, -2 );
    test_walk( 1486, 1768, -2,      1485, 1768,     true, -4 ); // on the gangplank
    test_walk( 1485, 1768, -4,      1485, 1769,     false, 0 ); // try to walk into the water
    test_walk( 1485, 1768, -4,      1485, 1767,     false, 0 ); // try to walk into the water
    test_walk( 1485, 1768, -4,      1484, 1768,     true, -2 );
    test_walk( 1487, 1765, -2,      1487, 1764,     true, -2 ); // try to walk onto the hold

    // try walking around on a boat in deeper water
    test_walk( 1497, 1781, -2,      1497, 1780,     true, -2 );
    test_walk( 1497, 1780, -2,      1497, 1779,     false, 0 ); // try to walk into the mast
    test_walk( 1497, 1780, -2,      1496, 1780,     true, -2 );
    test_walk( 1496, 1780, -2,      1496, 1779,     true, -2 );
    test_walk( 1496, 1779, -2,      1495, 1779,     true, -4 ); // on the gangplank
    test_walk( 1495, 1779, -4,      1495, 1778,     false, 0 ); // try to walk into the water
    test_walk( 1495, 1779, -4,      1495, 1780,     false, 0 ); // try to walk into the water
    test_walk( 1495, 1779, -4,      1494, 1779,     false, 0 );

    // try walking on a long boat, next to its plank
    test_walk( 1496, 1817, -2,      1495, 1817,     true, -2 );


    // walk down some stairs, um, somewhere
    test_walk( 5920,   40, 44,      5920,   41,     true, 41 );
    test_walk( 5920,   41, 41,      5920,   42,     true, 36 );
    test_walk( 5920,   42, 36,      5920,   43,     true, 31 );
    test_walk( 5920,   43, 31,      5920,   44,     true, 26 );
    test_walk( 5920,   44, 26,      5920,   45,     true, 22 );
    test_walk( 5920,   45, 22,      5921,   45,     true, 22 );
    test_walk( 5921,   45, 22,      5922,   45,     true, 22 );
    // walk up those same stairs
    test_walk( 5920,   45, 22,      5920,   44,     true, 26 );
    test_walk( 5920,   44, 26,      5920,   43,     true, 31 );
    test_walk( 5920,   43, 31,      5920,   42,     true, 36 );
    test_walk( 5920,   42, 36,      5920,   41,     true, 41 );
    test_walk( 5920,   41, 41,      5920,   40,     true, 44 );
    test_walk( 5920,   40, 44,      5920,   39,     true, 44 );
    test_walk( 5921,   40, 44,      5922,   40,     true, 44 );
    
    // static_debug_on = true;
    // walking onto a table?
    test_walk( 3664, 2589, 0,       3664, 2588,     false, 0 );
    // walking onto a table
    test_walk( 1594, 1588, 20,      1594, 1587,     false, 0 );

    test_walk( 1262, 1246, 0,       1262, 1245,     true,  0 );
    test_walk( 1264, 1253, 0,       1264, 1252,     true,  0 );

    test_walk( 1441, 1660, 9,       1442, 1660,     true,  10 /*9*/ );

    // near the britain bank,a steep walk
    test_walk( 1426, 1703, 3,       1425, 1704,     true, 9 );

    // walk up into an orc fort
    test_walk( 2163, 1330, 0,       2163, 1329,     true, 7 );

    // walk through the gates of Cove
    test_walk( 2284, 1209, 0,       2283, 1209,     true, 0 );

    // weird stairs in a dungeon
    test_walk( 5177, 188, 10,       5177, 187,      true, 20 );

    // wind is pissinhg me off...
    test_walk( 5166, 179, 17,       5166, 180,      true, 25 );

    // a steep grade in ocllo
    test_walk( 3773, 2654, 0,       3772, 2654,     true, 14 );

    // walk onto a teleporter
    test_walk( 1175, 2636, 0,       1175, 2635,     true, 0 );

    // walking on a peninsula
    test_walk( 1909, 51, 0,         1909,   50,     true, 0 );
    test_walk( 1909, 50, 0,         1910,   50,     true, -2 );
    test_walk( 1909, 50, 0,         1909,   49,     true, -1 );
    test_walk( 1909, 50, 0,         1908,   49,     true, -2 );
    test_walk( 1908, 49, -2,        1907,   49,     true, -3 );
    
        // This is known to fail - diagonal movement here should fail, but is allowed.
    test_walk( 1908, 51, 0,         1907,   50,     false, 0 );
    
    test_walk( 1908, 51, 0,         1908,   50,     true, 0 );
    test_walk( 1908, 50, 0,         1907,   50,     true, -1 );
    test_walk( 1909, 51, 0,         1908,   50,     true, 0 );
    test_walk( 1908, 50, 0,         1907,   49,     true, -3 );
    test_walk( 1907, 49, -3,        1906,   49,     true, -4 );
}

void test_multiwalk()
{
	cout << "UO datafile multi walk tests:" << endl;
    // now we have a small house in the hills
    test_walk( 2016, 1332, 0,       2016, 1331,     true, 4 );
    test_walk( 2016, 1331, 4,       2017, 1331,     true, 4 );
    test_walk( 2017, 1331, 4,       2018, 1331,     true, 0 );
    test_walk( 2018, 1331, 0,       2018, 1330,     false, 0 ); // walk N, blocked
    test_walk( 2018, 1331, 0,       2019, 1331,     true, 0 );
    test_walk( 2019, 1331, 0,       2019, 1330,     false, 0 );
    test_walk( 2019, 1331, 0,       2020, 1331,     true, 0 );
    test_walk( 2020, 1331, 0,       2020, 1330,     true, 0 );
    test_walk( 2016, 1331, 4,       2016, 1330,     true, 7 );
    test_walk( 2016, 1330, 7,       2015, 1330,     false, 0 );
    test_walk( 2016, 1330, 7,       2017, 1330,     false, 0 );
}
#endif

void pol_walk_test();
void pol_drop_test();
void pol_los_test();
void pol_test_multiwalk();
void display_test_results();

void run_pol_tests()
{
    create_test_environment();

    pol_drop_test();
    pol_walk_test();
    pol_los_test();
    pol_test_multiwalk();

#if ENABLE_OLD_MAPCODE
    walk_test();
    test_multiwalk();
    drop_test();
    map_test();
    statics_test();
    // skilladv_test();
    los_test();
#endif

    display_test_results();
}


/* TO ADD:
  (,5909,1395,30)  (walking through the floor?)
  (2193,1408,-2) -- no LOS to (2191, 1412, -2) ?
*/


#ifdef POLTEST
Item* Item::create( u16 objtype, u32 serial )
{
    passert( objtype < 0x4000 );
    return new Item(objtype, CLASS_ITEM);
}

UMulti* UMulti::create(  u16 objtype, u32 serial )
{
    UMulti* multi = NULL;
    if ( objtype < 0x4000 || objtype >= 0x5000)
        return NULL;

    if (defs[objtype-0x4000] != NULL)
    {
        multi = new UMulti( objtype );
    }
    else
    {
        cerr << "Tried to create multi " << objtype << " but no definition exists in MULTI.MUL" << endl;
        return NULL;
    }

/*
    if (serial)
        multi->serial = UseItemSerialNumber( serial );
    else
        multi->serial = GetNewItemSerialNumber();

    multi->serial_ext = ctBEu32( multi->serial );
*/
    
    return multi;
}


PolConfig config;
Items items;
Multis multis;

int main( int arc, char *argv[] )
{
    config.uo_datafile_root = "C:/Games/AoS2D/";
    cout << "Opening UO data files.." << endl;
    open_uo_data_files();
    read_uo_data();
    read_multidefs();

    run_pol_tests();
    return 0;
}
#endif
