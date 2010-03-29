/*
History
=======
2009/09/03 MuadDib:   Relocation of multi related cpp/h


Notes
=======

*/

#ifndef _CUSTOMHOUSES_H
#define _CUSTOMHOUSES_H

#include "../../clib/stl_inc.h"
#include "../../clib/cfgelem.h"
#include "../../clib/rawtypes.h"

#include "../network/client.h"
#include "../pktboth.h"

#define CUSTOM_HOUSE_NUM_PLANES 5

class UHouse;

enum{
    HOUSE_DESIGN_CURRENT = 1,
    HOUSE_DESIGN_WORKING
};

struct CUSTOM_HOUSE_ELEMENT
{
    u16 graphic;
    s32 xoffset;
    s32 yoffset;
    u8 z;
};

typedef vector<vector<list<CUSTOM_HOUSE_ELEMENT> > > HouseFloor;//vector of N-S rows
typedef vector<list<CUSTOM_HOUSE_ELEMENT> > HouseFloorRow;//vector of Z columns
typedef list<CUSTOM_HOUSE_ELEMENT> HouseFloorZColumn;
    // [][][][][][] - Xoffset vector
    // | | | | | |  - vec of yoffsets
    // v v v v v v
    // [][][][][][] - lists of zoffsets
    // [][][][][][]

class CustomHouseElements
{
public:
    CustomHouseElements(){};
    CustomHouseElements(u32 _height,u32 _width, s32 xoffset, s32 yoffset) :
        height(_height),
        width(_width),
        xoff(xoffset),
        yoff(yoffset)
    {
        SetWidth(_width);
        SetHeight(_height);
    };
    ~CustomHouseElements(){};

    void SetHeight(u32 _height)
    {
        height = _height;
        for(unsigned int i=0; i<width; i++)
            data.at(i).resize(height);
    }
    void SetWidth(u32 _width)
    {
        width = _width;
        data.resize(width);
    }

    HouseFloorZColumn* GetElementsAt(s32 xoffset, s32 yoffset);

    void AddElement(CUSTOM_HOUSE_ELEMENT& elem);

    HouseFloor data; 
    u32 height, width;
    s32 xoff, yoff;

};

class CustomHouseDesign
{
public:
    CustomHouseDesign();
    CustomHouseDesign(u32 _height, u32 _width, s32 xoffset, s32 yoffset);
    ~CustomHouseDesign();
    void InitDesign(u32 _height, u32 _width, s32 xoffset, s32 yoffset);

    void operator=(CustomHouseDesign& design);
    void Add(CUSTOM_HOUSE_ELEMENT& elem);
    void AddOrReplace(CUSTOM_HOUSE_ELEMENT& elem);
    void AddMultiAtOffset(u16 graphic, s8 x, s8 y, s8 z);

    bool Erase(u32 xoffset, u32 yoffset, u8 z, int minheight = 0);
    bool EraseGraphicAt(u16 graphic, u32 xoffset, u32 yoffset, u8 z);

    void ReplaceDirtFloor(u32 x, u32 y);
    void Clear();
    bool IsEmpty() const;

    unsigned char* Compress(int floor, u32* uncompr_length, u32* compr_length);
    
    unsigned int TotalSize() const;
    unsigned char NumUsedPlanes() const;
    
    void readProperties( ConfigElem& elem, const string prefix );
    void printProperties( ostream& os, const string prefix ) const;

    int floor_sizes[CUSTOM_HOUSE_NUM_PLANES];

    bool DeleteStairs( u16 id, s32 x, s32 y, s8 z );

    //assumes x,y already added with xoff and yoff
    inline bool ValidLocation( u32 xidx, u32 yidx )
    {
        return !(xidx >= width || yidx >= height || xidx < 0 || yidx < 0);
    }

    static bool IsStair( u16 id, int& dir );
    static bool IsStairBlock( u16 id );

    u32 height, width; //total sizes including front steps
    s32 xoff, yoff;      //offsets from center west-most and north-most indicies are 0
    CustomHouseElements Elements[CUSTOM_HOUSE_NUM_PLANES]; //5 planes

    static const char custom_house_z_xlate_table[CUSTOM_HOUSE_NUM_PLANES];
    //for testing
    void testprint( ostream& os ) const;
private:
    char z_to_custom_house_table(char z);
};

//House Tool Command Implementations:
void CustomHousesAdd(PKTBI_D7* msg);
void CustomHousesAddMulti(PKTBI_D7* msg);
void CustomHousesErase(PKTBI_D7* msg);
void CustomHousesClear(PKTBI_D7* msg);
void CustomHousesQuit(PKTBI_D7* msg);
void CustomHousesCommit(PKTBI_D7* msg);
void CustomHousesSelectFloor(PKTBI_D7* msg);
void CustomHousesBackup(PKTBI_D7* msg);
void CustomHousesRestore(PKTBI_D7* msg);
void CustomHousesSynch(PKTBI_D7* msg);
void CustomHousesRevert(PKTBI_D7* msg);
void CustomHousesSendFull(UHouse* house, Client* client, int design = HOUSE_DESIGN_CURRENT);
void CustomHousesSendFullToInRange(UHouse* house, int design, int range);
void CustomHousesSendShort(UHouse* house, Client* client);
void CustomHouseStopEditing(Character* chr, UHouse* house);
#endif
