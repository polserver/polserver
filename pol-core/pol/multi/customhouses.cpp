/*
History
=======
2005/08/19 Shinigami: ZLib functionality linked directly into Core
2005/09/26 Shinigami: wrong styled break condition in ::Compress
2009/09/03 MuadDib:   Relocation of multi related cpp/h
2009/12/02 Turley:    added config.max_tile_id - Tomi


Notes
=======

TODO:
Will require a special Character member to check in all kinds of places. Character.EditHouse=0/1
The purpose, is to make NPCs IGNORE THIS PERSON, and to NOT send ANY packets about this person
to ANYONE. This will also need to be checked when using all sorts of crap. Like when triggering
equip/insert/item use/etc. This char should be able to do NOTHING except edit the house when
in edit house mode, etc.
Enabling this mode can be internal only, simply by linking it with the Custom housing tool.
Have a script that can be made to run additional stuff with using the tool, so scripters can
check extra stuff if they want, etc, and that script's return decides if to activate the housing
tool. Should suffice.

*/

#include "../../clib/stl_inc.h"
#include "../../clib/cfgelem.h"
#include "../../clib/endian.h"
#include "../../clib/stlutil.h"

#ifdef USE_SYSTEM_ZLIB
#	include <zlib.h>
#else
#	include "../../../lib/zlib/zlib.h"
#endif

#include "../mobile/charactr.h"
#include "../network/cgdata.h"
#include "../network/client.h"
#include "../network/packets.h"
#include "../network/clienttransmit.h"
#include "../core.h"
#include "../uvars.h"
#include "house.h"
#include "customhouses.h"
#include "../fnsearch.h"
#include "multi.h"
#include "multidef.h"
#include "../polcfg.h"
#include "../pktboth.h"
#include "../ufunc.h"
#include "../ustruct.h"
#include "../uworld.h"
#include "../item/itemdesc.h"
#include "../syshook.h"
#include "../mkscrobj.h"
#include "../scrsched.h"

//bytes per tile - currently only mode 0 works, meaning we send u16 graphic, s8 x,y,z offsets
#define BYTES_PER_TILE 5

//fixed z offsets for each floor
const char CustomHouseDesign::custom_house_z_xlate_table[CUSTOM_HOUSE_NUM_PLANES] = {0,7,27,47,67}; 

//translate z offset to floor number, use floor below passed-in z value, unless exact match
char CustomHouseDesign::z_to_custom_house_table(char z)
{
    unsigned char i;
    for( i=0; i<CUSTOM_HOUSE_NUM_PLANES; i++)
	{
        if( z == custom_house_z_xlate_table[i] )
            return i;
        else if( z < custom_house_z_xlate_table[i] )
            return i-1;
	}
    return -1;
}

CustomHouseDesign::CustomHouseDesign()
{
    for(int i=0;i<CUSTOM_HOUSE_NUM_PLANES;i++)
        floor_sizes[i] = 0;
}

//fixme: need a copy ctor?

CustomHouseDesign::CustomHouseDesign(u32 _height, u32 _width, s32 xoffset, s32 yoffset)
{
    InitDesign(_height,_width,xoffset,yoffset);
}

CustomHouseDesign::~CustomHouseDesign()
{
}

//init the geometry of the design. the design cannot exist outside the multi foundation boundary (exception: front steps)
void CustomHouseDesign::InitDesign(u32 _height,u32 _width, s32 xoffset, s32 yoffset)
{
    height = _height;
    width = _width;
    xoff = xoffset;
    yoff = yoffset;
    for(int i=0;i<CUSTOM_HOUSE_NUM_PLANES;i++)
    {
        floor_sizes[i] = 0;
        Elements[i].SetWidth(_width);
        Elements[i].SetHeight(_height);
        Elements[i].xoff = xoffset;
        Elements[i].yoff = yoffset;
    }
}

void CustomHouseDesign::operator=(CustomHouseDesign& design)
{
    for(int i = 0; i<CUSTOM_HOUSE_NUM_PLANES; i++)
    {
        Elements[i] = design.Elements[i];
        floor_sizes[i] = design.floor_sizes[i];
    }
}

//Adds the element to the design
void CustomHouseDesign::Add(CUSTOM_HOUSE_ELEMENT& elem)
{
    int floor_num = z_to_custom_house_table(elem.z);
	if ( floor_num == -1 )
		return;
    Elements[floor_num].AddElement(elem);
    floor_sizes[floor_num]++;
}

//fixme: low walls not being replaced
//Replaces an existing object depending on the 2 tile heights
void CustomHouseDesign::AddOrReplace(CUSTOM_HOUSE_ELEMENT& elem)
{
    int floor_num = z_to_custom_house_table(elem.z);
	if ( floor_num == -1 )
		return;
    char adding_height = tileheight(elem.graphic);

    int xidx = elem.xoffset + xoff;
    int yidx = elem.yoffset + yoff;
    if(!ValidLocation(xidx,yidx))
        return;
    for( HouseFloorZColumn::iterator itr = Elements[floor_num].data.at(xidx).at(yidx).begin(),
		 itrend = Elements[floor_num].data.at(xidx).at(yidx).end(); 
         itr != itrend; ++itr)
    {
            char existing_height = tileheight(itr->graphic);
            
            if( ((existing_height == 0) && (adding_height == 0)) || //replace floor with floor
                ((existing_height != 0) && (adding_height != 0)) )  //or nonfloor with nonfloor
                                                                   
            {
                Elements[floor_num].data.at(xidx).at(yidx).erase(itr);
                floor_sizes[floor_num]--;
                Add(elem);
                return;
            }
    }

    //no replacement, just add
    Add(elem);
}

bool CustomHouseDesign::Erase(u32 xoffset, u32 yoffset, u8 z, int minheight)
{
    int floor_num = z_to_custom_house_table(z);
	if ( floor_num == -1 )
		return false;
  
    int xidx = xoffset + xoff;
    int yidx = yoffset + yoff;
    if(!ValidLocation(xidx,yidx))
        return false;
    for( HouseFloorZColumn::iterator itr = Elements[floor_num].data.at(xidx).at(yidx).begin(),
		 itrend = Elements[floor_num].data.at(xidx).at(yidx).end(); 
         itr != itrend; ++itr)
    {
        char height = tileheight(itr->graphic);
        if( (itr->z == z) && (height >= minheight) )
        {
            Elements[floor_num].data.at(xidx).at(yidx).erase(itr);
            floor_sizes[floor_num]--;
            return true;
        }
    }
    return false;
}


bool CustomHouseDesign::EraseGraphicAt(u16 graphic, u32 xoffset, u32 yoffset, u8 z)
{
    int floor_num = z_to_custom_house_table(z);
	if ( floor_num == -1 )
		return false;

    int xidx = xoffset + xoff;
    int yidx = yoffset + yoff;
    if(!ValidLocation(xidx,yidx))
        return false;
    for( HouseFloorZColumn::iterator itr = Elements[floor_num].data.at(xidx).at(yidx).begin(),
		 itrend = Elements[floor_num].data.at(xidx).at(yidx).end(); 
         itr != itrend; ++itr)
    {
        if( itr->graphic == graphic )
        {
            Elements[floor_num].data.at(xidx).at(yidx).erase(itr);
            floor_sizes[floor_num]--;
            return true;
        }
    }
    return false;
}

void CustomHouseDesign::ReplaceDirtFloor(u32 x, u32 y)
{
    int floor_num = 1; //dirt always goes on floor 1 (z=7)

    if(x+xoff == 0 || y+yoff == 0) //don't replace dirt at far-west and far-north sides
        return;

    bool floor_exists = false;

    int xidx = x + xoff;
    int yidx = y + yoff;
    if(!ValidLocation(xidx,yidx))
        return;
    for( HouseFloorZColumn::iterator itr = Elements[floor_num].data.at(xidx).at(yidx).begin(),
		 itrend = Elements[floor_num].data.at(xidx).at(yidx).end(); 
         itr != itrend; ++itr)
    {
        if( tileheight(itr->graphic) == 0 ) //a floor tile exists
        {
            floor_exists = true;
			break;
        }
    }

    if( floor_exists == false ) //add a dirt tile 
    {
        CUSTOM_HOUSE_ELEMENT elem;
        elem.graphic = DIRTY_TILE; 
        elem.xoffset = x;
        elem.yoffset = y;
        elem.z = 7;

        Add(elem);
    }
}

void CustomHouseDesign::Clear()
{
    //delete contents of all z column lists
    for(int i=0; i<CUSTOM_HOUSE_NUM_PLANES; i++)
    {
        for(HouseFloor::iterator xitr = Elements[i].data.begin(),
			                     xitrend = Elements[i].data.end(); 
								 xitr != xitrend; ++xitr)
        {
            for(HouseFloorRow::iterator yitr = xitr->begin(),
				                        yitrend = xitr->end(); 
										yitr != yitrend; ++yitr)
            {
                yitr->clear();
            }
        }
        floor_sizes[i] = 0;
    }
}

//caller must delete, assume type 0
unsigned char* CustomHouseDesign::Compress(int floor, u32* uncompr_length, u32* compr_length)
{
    int numtiles = floor_sizes[floor];
    int nextindex = 0;
    unsigned int ubuflen = numtiles*BYTES_PER_TILE;
    unsigned long cbuflen = (((unsigned long)(( (float)(ubuflen) )*1.001f)) +12);//as per zlib spec
    unsigned char* uncompressed = new unsigned char[ubuflen];
    memset(uncompressed,0,ubuflen);
    unsigned char* compressed = new unsigned char[cbuflen]; 
    memset(compressed,0,cbuflen);

	int i = 0;
    for(HouseFloor::const_iterator xitr = Elements[floor].data.begin(),
		                           xitrend = Elements[floor].data.end();
	                               xitr != xitrend; ++xitr)
    {
		i = 0;
        for(HouseFloorRow::const_iterator yitr = xitr->begin(),
			                              yitrend = xitr->end(); 
										  yitr != yitrend; ++yitr)
        {
            for(HouseFloorZColumn::const_iterator zitr = yitr->begin(),
				                                  zitrend = yitr->end(); 
												  zitr != zitrend; ++zitr, ++i)
            {
                //assume type 0, I don't know how to deal with stair pieces at odd Z values for mode 1,
                //and mode 2 is just wacky. (position implied from list position, needs alot of null tiles
                //to make that work (but they compress very well)
				if  (i < numtiles)
				{
					uncompressed[nextindex++] = (u8)((zitr->graphic >> 8) & 0xFF);
					uncompressed[nextindex++] = (u8)(zitr->graphic & 0xFF);
        
					uncompressed[nextindex++] = (u8)zitr->xoffset;
					uncompressed[nextindex++] = (u8)zitr->yoffset;
					uncompressed[nextindex++] = (u8)zitr->z;
				}
            }
        }
    }
    *uncompr_length = nextindex;

    int ret = compress2 ( compressed, &cbuflen, uncompressed, nextindex, Z_DEFAULT_COMPRESSION); 
    if(ret == Z_OK)
    {
        delete[] uncompressed;
        *compr_length = cbuflen;
        return compressed;
    }
    else
    {
        *uncompr_length = 0;
        *compr_length = 0;
        delete[] compressed;
        if(ubuflen > 0)
            delete[] uncompressed;
        return NULL;
    }
}

bool CustomHouseDesign::IsEmpty() const
{
    int total = 0;
    for(int i=0; i<CUSTOM_HOUSE_NUM_PLANES; i++)
        total += floor_sizes[i];
    return total == 0 ? true : false;
}

unsigned int CustomHouseDesign::TotalSize() const
{
    unsigned int size = 0;
    for(int i=0; i<CUSTOM_HOUSE_NUM_PLANES; i++)
    {
        size += floor_sizes[i];
    }
    return size;
}

unsigned char CustomHouseDesign::NumUsedPlanes() const
{
    unsigned char size = 0;
    for(int i=0; i<CUSTOM_HOUSE_NUM_PLANES; i++)
    {
        if(floor_sizes[i] > 0)
            size++;
    }
    return size;
}

//used to add the base foundation to initialize a design, and multi stairs. not tested with anything else.
//I guess you could add a boat or something inside a house, but deleting would be impossible. 
//Deleting stairs is handled explicitly (CustomHouseDesign::DeleteStairs).
void CustomHouseDesign::AddMultiAtOffset(u16 graphic, s8 x, s8 y, s8 z)
{
    const MultiDef* multidef = MultiDefByGraphic(graphic);
    if(multidef == NULL)
    {
        cerr << "Trying to add Multi to customhouse, graphic " << hex << graphic << dec << " multi definition doesn't exist!" << endl;
        return;
    }

    for( MultiDef::Components::const_iterator itr = multidef->components.begin(), end = multidef->components.end();
         itr != end;
         ++itr )
    {
        const MULTI_ELEM* m_elem = itr->second;
        if( ((m_elem->objtype) & config.max_tile_id) == 1 ) //don't add the invisible multi tile
            continue;
        //cout << "0x" << hex << m_elem->graphic
        //     << " 0x" << hex << m_elem->flags 
        //     << ":" << dec <<  m_elem->x << "," << m_elem->y << "," << m_elem->z << endl;
        CUSTOM_HOUSE_ELEMENT ch_elem;
        ch_elem.graphic = m_elem->objtype;
        ch_elem.xoffset = m_elem->x + x;
        ch_elem.yoffset = m_elem->y + y;
        ch_elem.z = static_cast<u8>(m_elem->z + z);
        Add(ch_elem);
    }
}

void CustomHouseDesign::readProperties( ConfigElem& elem, const string& prefix  )
{
    string line, sizes;

    while (elem.remove_prop( prefix.c_str(), &line ))
    {
        ISTRINGSTREAM is( line );
        u16 graphic;
        s32 x,y;
        u16 z;
        is >> graphic;
        is >> x;
        is >> y;
        is >> z;
        
        CUSTOM_HOUSE_ELEMENT elem;
        elem.graphic = graphic;
        elem.xoffset = x;
        elem.yoffset = y;
        elem.z = (u8)z;
        Add(elem);
    }
}

void CustomHouseDesign::printProperties( ostream& os, const string& prefix ) const
{
    if(!IsEmpty())
    {
        for( int i=0; i<CUSTOM_HOUSE_NUM_PLANES; i++ )
        {
            for(HouseFloor::const_iterator xitr = Elements[i].data.begin(),
				                           xitrend = Elements[i].data.end(); 
										   xitr != xitrend; ++xitr)
            {
                for(HouseFloorRow::const_iterator yitr = xitr->begin(),
					                              yitrend = xitr->end(); 
												  yitr != yitrend; ++yitr)
                {
                    for(HouseFloorZColumn::const_iterator zitr = yitr->begin(),
						                                  zitrend = yitr->end(); 
														  zitr != zitrend; ++zitr)
                    {
                        os << "\t" << prefix << "\t " << zitr->graphic << " " << zitr->xoffset << " " << zitr->yoffset << " " << (u16)zitr->z << endl;
                    }
                }
            }
        }
    }
}

//for testing, prints each floor's x,y,z rows
void CustomHouseDesign::testprint( ostream& os ) const
{
    if(!IsEmpty())
    {
        for( int i=0; i<CUSTOM_HOUSE_NUM_PLANES; i++ )
        {
            int x=0, y=0;
			for( HouseFloor::const_iterator xitr = Elements[i].data.begin(),
				                           xitrend = Elements[i].data.end(); 
										   xitr != xitrend; ++xitr,x++)
			{
                os << "X: " << x << endl;
				for( HouseFloorRow::const_iterator yitr = xitr->begin(),
					                               yitrend = xitr->end(); 
					                               yitr != yitrend; ++yitr,y++)
				{
                    os << "\tY: " << y << endl;
					for( HouseFloorZColumn::const_iterator zitr = yitr->begin(),
						                                   zitrend = yitr->end(); 
						                                   zitr != zitrend; ++zitr)
					{
                        os << "\t\t" << zitr->graphic << " " << zitr->xoffset << " " << zitr->yoffset << " " << (u16)zitr->z << endl;
                    }
                }
            }
        }
    }
}

void CustomHouseDesign::ClearComponents( UHouse* house )
{
	UHouse::Components* comp = house->get_components();
	UHouse::Components::iterator itr = comp->begin();
	while (itr != comp->end())
	{
		Item* item = (*itr).get();
		if (item != NULL && !item->orphan())
		{
			if (!item->invisible()) //give scripters the chance to keep an item alive
			{
				itr = comp->erase(itr);
				destroy_item(item);
				continue;
			}
		}
		++itr;
	}
}

void CustomHouseDesign::AddComponents( UHouse* house )
{
	UHouse::Components* comp = house->get_components();
	for( UHouse::Components::const_iterator itr = comp->begin(), end = comp->end(); itr != end; ++itr )
	{
		Item* item = (*itr).get();
		if (item != NULL && !item->orphan())
		{
			if (!item->invisible()) //give scripters the chance to keep an item alive
			{
				CUSTOM_HOUSE_ELEMENT elem;
				elem.graphic = item->graphic;
				elem.xoffset = item->x - house->x;
				elem.yoffset = item->y - house->y;
				elem.z = item->z - house->z;
				Add(elem);
			}
		}
	}

}
void CustomHouseDesign::FillComponents( UHouse* house, bool add_as_component )
{
	for(int i=0; i<CUSTOM_HOUSE_NUM_PLANES; i++)
	{
		for(HouseFloor::iterator xitr = Elements[i].data.begin(),
			xitrend = Elements[i].data.end(); 
			xitr != xitrend; ++xitr)
		{
			for(HouseFloorRow::iterator yitr = xitr->begin(),
				yitrend = xitr->end(); 
				yitr != yitrend; ++yitr)
			{
				HouseFloorZColumn::iterator zitr = yitr->begin();
				while( zitr != yitr->end() )
				{
					const ItemDesc& id = find_itemdesc( zitr->graphic );
					if (id.type == ItemDesc::DOORDESC)
					{
						if (add_as_component)
						{
							Item* component = Item::create( id.objtype );
							if (component != NULL) 
								house->add_component(component, zitr->xoffset, zitr->yoffset, zitr->z);
						}
						zitr = yitr->erase(zitr);
						floor_sizes[i]--;
					}
					else if (zitr->graphic >= TELEPORTER_START && zitr->graphic <= TELEPORTER_END ) // teleporters
					{
						if (add_as_component)
						{
							Item* component = Item::create( zitr->graphic );
							if (component != NULL)
								house->add_component(component, zitr->xoffset, zitr->yoffset, zitr->z);
						}
						zitr = yitr->erase(zitr);
						floor_sizes[i]--;
					}
					else
						++zitr;
				}
			}
		}
	}
}

ObjArray* CustomHouseDesign::list_parts() const
{
	auto_ptr<ObjArray> arr (new ObjArray);
	for(int i=0; i<CUSTOM_HOUSE_NUM_PLANES; i++)
	{
		for( HouseFloor::const_iterator xitr = Elements[i].data.begin(),
			xitrend = Elements[i].data.end(); 
			xitr != xitrend; ++xitr)
		{
			for( HouseFloorRow::const_iterator yitr = xitr->begin(),
				yitrend = xitr->end(); 
				yitr != yitrend; ++yitr)
			{
				for( HouseFloorZColumn::const_iterator zitr = yitr->begin(),
					zitrend = yitr->end(); 
					zitr != zitrend; ++zitr)
				{
					auto_ptr<BStruct> itemstruct (new BStruct);
					itemstruct->addMember("graphic",new BLong(zitr->graphic));
					itemstruct->addMember("xoffset",new BLong(zitr->xoffset));
					itemstruct->addMember("yoffset",new BLong(zitr->yoffset));
					itemstruct->addMember("z",new BLong(zitr->z));
					arr->addElement(itemstruct.release());
				}
			}
		}
	}
	return arr.release();
}

void CustomHouseStopEditing(Character* chr, UHouse* house)
{
	PktOut_BF_Sub20* msg = REQUESTSUBPACKET(PktOut_BF_Sub20,PKTBI_BF_ID,PKTBI_BF::TYPE_ACTIVATE_CUSTOM_HOUSE_TOOL);
	msg->WriteFlipped(static_cast<u16>(17));
	msg->offset+=2; //sub
	msg->Write(house->serial_ext);
	msg->Write(static_cast<u8>(0x5)); //end
	msg->offset+=2; // u16 unk2 FIXME what's the meaning
	msg->Write(static_cast<u32>(0xFFFFFFFF)); // fixme
	msg->Write(static_cast<u8>(0xFF)); // fixme
    ADDTOSENDQUEUE(chr->client,&msg->buffer,msg->offset);
	READDPACKET(msg);

	const MultiDef& def = house->multidef();
    move_character_to(chr,house->x+def.minrx,house->y+def.maxry+1,house->z,MOVEITEM_FORCELOCATION, NULL);
    chr->client->gd->custom_house_serial = 0;
    house->editing = false;
	ItemList itemlist;
	MobileList moblist;
	UHouse::list_contents( house, itemlist, moblist );
	while (!itemlist.empty())
	{
		Item* item = itemlist.front();
		send_item( chr->client, item );
		itemlist.pop_front();
	}
}

void CustomHousesAdd(PKTBI_D7* msg)
{
    u32 serial = cfBEu32(msg->serial);
    UHouse* house = UHouse::FindWorkingHouse(serial);
    if(house == NULL)
        return;

    CH_ADD add = msg->ch_add;
    CUSTOM_HOUSE_ELEMENT elem;
    elem.graphic = cfBEu16(add.tileID);
    elem.xoffset = cfBEu32(add.xoffset);
    elem.yoffset = cfBEu32(add.yoffset);
    elem.z = CustomHouseDesign::custom_house_z_xlate_table[house->editing_floor_num];

    //the south side of the house can have stairs at z=0
    int ysize = house->multidef().maxry - house->multidef().minry;
    if( (elem.yoffset + (ysize/2)) == ysize+1 )
        elem.z = 0;

    house->WorkingDesign.AddOrReplace(elem);

    //invalidate stored packet
    vector<u8> newvec;
    house->WorkingCompressed.swap(newvec);

    house->revision++;
}

void CustomHousesAddMulti(PKTBI_D7* msg)
{
    u32 serial = cfBEu32(msg->serial);
    UHouse* house = UHouse::FindWorkingHouse(serial);
    if(house == NULL)
        return;

    u16 itemID = cfBEu16(msg->ch_add_multi.multiID);
    s8 x = static_cast<s8>(cfBEu32(msg->ch_add_multi.xoffset)), y = static_cast<s8>(cfBEu32(msg->ch_add_multi.yoffset));
    s8 z = CustomHouseDesign::custom_house_z_xlate_table[house->editing_floor_num];

    //only allow stairs IDs
    if ( itemID < STAIR_MULTIID_MIN || itemID > STAIR_MULTIID_MAX )
    {
		Character* chr = find_character(serial);
		if(chr && chr->client)
			CustomHousesSendFull(house, chr->client,HOUSE_DESIGN_WORKING);
        return;
    }

    house->WorkingDesign.AddMultiAtOffset(itemID,x,y,z);

    //invalidate stored packet
    vector<u8> newvec;
    house->WorkingCompressed.swap(newvec);

    house->revision++;
}

void CustomHousesErase(PKTBI_D7* msg)
{
    u32 serial = cfBEu32(msg->serial);
    UHouse* house = UHouse::FindWorkingHouse(serial);
    if(house == NULL)
        return;

    u32 x = cfBEu32(msg->ch_erase.xoffset),y = cfBEu32(msg->ch_erase.yoffset);
    u8 z = static_cast<u8>(cfBEu32(msg->ch_erase.z));
    u16 graphic = cfBEu16(msg->ch_erase.tileID);

	u32 realx = x + house->WorkingDesign.xoff;
	u32 realy = y + house->WorkingDesign.yoff;
	if( z == 0 && realx >= 0 && realx < house->WorkingDesign.width && realy >= 0 && realy < (house->WorkingDesign.height - 1) )
	{
		Character* chr = find_character(serial);
		if(chr && chr->client)
			CustomHousesSendFull(house, chr->client,HOUSE_DESIGN_WORKING);
		return;
	}

    //check if not deleting a stairs piece (if we are, DeleteStairs will do it)
    if ( !house->WorkingDesign.DeleteStairs( graphic, x, y, z ) )
    {
		house->WorkingDesign.EraseGraphicAt( graphic, x, y, z ); 
        //maybe replace empty ground floor with dirt tile
        if(z == CustomHouseDesign::custom_house_z_xlate_table[1])
            house->WorkingDesign.ReplaceDirtFloor(x,y);
    }

    //invalidate stored packet
    vector<u8> newvec;
    house->WorkingCompressed.swap(newvec);

    house->revision++;

}

void CustomHousesClear(PKTBI_D7* msg)
{
    u32 serial = cfBEu32(msg->serial);
    Character* chr = find_character(serial);
    UHouse* house = UHouse::FindWorkingHouse(serial);
    if(house == NULL)
        return;

    house->WorkingDesign.Clear();

    //invalidate stored packet
    vector<u8> newvec;
    house->WorkingCompressed.swap(newvec);

    //add foundation back to design
    house->WorkingDesign.AddMultiAtOffset(house->graphic,0,0,0);
    if(chr && chr->client)
        CustomHousesSendFull(house, chr->client,HOUSE_DESIGN_WORKING);

    house->revision++;
}

//if the client closed his tool
void CustomHousesQuit(PKTBI_D7* msg)
{
    u32 serial = cfBEu32(msg->serial);
    UHouse* house = UHouse::FindWorkingHouse(serial);
    if(house == NULL)
        return;
	house->CurrentDesign.FillComponents(house);
	house->WorkingDesign.FillComponents(house,false); // keep in sync
	house->revision++;
	vector<u8> newvec;
	house->WorkingCompressed.swap(newvec);

	vector<u8> newvec2;
	house->CurrentCompressed.swap(newvec2);
	Character* chr = find_character(serial);


    if(chr && chr->client)
    {
        CustomHouseStopEditing(chr,house);
        CustomHousesSendFull(house, chr->client,HOUSE_DESIGN_CURRENT);
    }
}

void CustomHousesCommit(PKTBI_D7* msg)
{
    u32 serial = cfBEu32(msg->serial);
    UHouse* house = UHouse::FindWorkingHouse(serial);
    Character* chr = find_character(serial);
    if(house == NULL || chr == NULL)
        return;

    //remove dynamic bits (teleporters, doors)
	house->WorkingDesign.FillComponents(house);
	
	//call a script to do post processing (calc cost, yes/no confirm, consume cost, link teleporters)
	ScriptDef sd;
	if (sd.config_nodie( "misc/customhousecommit.ecl", 0, 0 ))
	{
		if ( sd.exists() )
		{
			house->waiting_for_accept = true;
			if ( start_script( sd, make_mobileref(chr), new EMultiRefObjImp(house),house->WorkingDesign.list_parts() ) != NULL )
				return;
		}
	}
	house->AcceptHouseCommit(chr,true);
}

void CustomHousesSelectFloor(PKTBI_D7* msg)
{
    u32 serial = cfBEu32(msg->serial);
    UHouse* house = UHouse::FindWorkingHouse(serial);
    Character* chr = find_character(serial);
    if(house == NULL)
        return;
    u8 floor = msg->ch_select_floor.floornumber;

    if(floor < 1 || floor > 4)
        floor = 1;

    house->editing_floor_num = floor;

	if (chr)
	{
		move_character_to(chr, chr->x, chr->y, house->z + CustomHouseDesign::custom_house_z_xlate_table[floor],MOVEITEM_FORCELOCATION, NULL);
		if(chr->client)
			CustomHousesSendShort(house,chr->client);
	}
}

void CustomHousesBackup(PKTBI_D7* msg)
{
    u32 serial = cfBEu32(msg->serial);
    UHouse* house = UHouse::FindWorkingHouse(serial);
    if(house == NULL)
        return;

    house->BackupDesign = house->WorkingDesign;
}

void CustomHousesRestore(PKTBI_D7* msg)
{
    u32 serial = cfBEu32(msg->serial);
    UHouse* house = UHouse::FindWorkingHouse(serial);
    Character* chr = find_character(serial);
    if(house == NULL)
        return;

    house->WorkingDesign = house->BackupDesign;
    vector<u8> newvec;
    house->WorkingCompressed.swap(newvec);
    if(chr && chr->client)
        CustomHousesSendFull(house, chr->client,HOUSE_DESIGN_WORKING);
}

void CustomHousesSynch(PKTBI_D7* msg)
{
    u32 serial = cfBEu32(msg->serial);
    Character* chr = find_character(serial);
    UHouse* house = UHouse::FindWorkingHouse(serial);
    if(chr && chr->client)
        CustomHousesSendFull(house, chr->client,HOUSE_DESIGN_WORKING);
}

//replace working design with currently committed design
void CustomHousesRevert(PKTBI_D7* msg)
{
    u32 serial = cfBEu32(msg->serial);
    UHouse* house = UHouse::FindWorkingHouse(serial);
    
    if(house == NULL)
        return;

    house->WorkingDesign = house->CurrentDesign;
    vector<u8> newvec;
    house->WorkingCompressed.swap(newvec);
	Character* chr = find_character(serial);
    if(chr && chr->client)
        CustomHousesSendFull(house, chr->client,HOUSE_DESIGN_WORKING);
}

void CustomHousesRoofSelect(PKTBI_D7* msg)
{
	u32 serial = cfBEu32(msg->serial);
	UHouse* house = UHouse::FindWorkingHouse(serial);
	if(house == NULL)
		return;

	CH_SELECT_ROOF add = msg->ch_select_roof;
	CUSTOM_HOUSE_ELEMENT elem;
	elem.graphic = cfBEu16(add.tileID);
	elem.xoffset = cfBEu32(add.xoffset);
	elem.yoffset = cfBEu32(add.yoffset);
	s8 z = static_cast<s8>(cfBEu32(add.zoffset));
	if( z < -3 || z > 12 || z % 3 != 0 )
		z = -3;
	elem.z = z + CustomHouseDesign::custom_house_z_xlate_table[house->editing_floor_num];

	house->WorkingDesign.AddOrReplace(elem);

	//invalidate stored packet
	vector<u8> newvec;
	house->WorkingCompressed.swap(newvec);

	house->revision++;
}
void CustomHousesRoofRemove(PKTBI_D7* msg)
{
	u32 serial = cfBEu32(msg->serial);
	UHouse* house = UHouse::FindWorkingHouse(serial);
	if(house == NULL)
		return;

	CH_DELETE_ROOF remove = msg->ch_delete_roof;
	u32 x = cfBEu32(remove.xoffset),y = cfBEu32(remove.yoffset);
	u8 z = static_cast<u8>(cfBEu32(remove.zoffset));
	u16 graphic = cfBEu16(remove.tileID);
	if (!house->WorkingDesign.EraseGraphicAt(graphic,x,y,z))
	{
		Character* chr = find_character(serial);
		if(chr && chr->client)
			CustomHousesSendFull(house, chr->client,HOUSE_DESIGN_WORKING);
		return;
	}

	//invalidate stored packet
	vector<u8> newvec;
	house->WorkingCompressed.swap(newvec);

	house->revision++;
}

void CustomHousesSendFull(UHouse* house, Client* client, int design)
{
    u32 clen;
    u32 ulen;
    unsigned char* data;
    //unsigned char** stored_packet;
    vector<u8>* stored_packet;

    u32 planeheader = 0;
    u32 buffer_len = 0;
    CustomHouseDesign* pdesign;
    const unsigned int data_offset = 17;
    const u32 mode = 0; //we only know how to do mode 0 at this point.

    //choose between sending working or current designs
    switch(design)
    {
        case HOUSE_DESIGN_CURRENT:
            if(house->CurrentCompressed.empty()) //no design stored, create below
            {
                pdesign = &house->CurrentDesign; 
                stored_packet = &house->CurrentCompressed;
            }
            else //send stored packet
            {
                ADDTOSENDQUEUE(client, &house->CurrentCompressed[0],  ctBEu16( *( reinterpret_cast<u16*>(&house->CurrentCompressed[1])) ) );
                return;
            }
            break;
        case HOUSE_DESIGN_WORKING:
            if(house->WorkingCompressed.empty()) //no design stored, create below
            {
                pdesign = &house->WorkingDesign; 
                stored_packet = &house->WorkingCompressed;
            }
            else //send stored packet
            {
                ADDTOSENDQUEUE(client, &house->WorkingCompressed[0],  ctBEu16( *( reinterpret_cast<u16*>(&house->WorkingCompressed[1])) ) );
                return;
            }
            break;
        default: return;
    }

    //create compressed house message
   
    unsigned char planes = pdesign->NumUsedPlanes();
    unsigned long sbuflen = ((unsigned long)(((float)(pdesign->TotalSize()*BYTES_PER_TILE))*1.001f)+12);
    sbuflen += planes*BYTES_PER_TILE; //for plane header dword
    sbuflen += 17; //for packet header

    vector<u8> packet(sbuflen);


    PKTOUT_D8* msg = reinterpret_cast<PKTOUT_D8*>(&packet[0]);
    msg->msgtype = PKTOUT_D8_ID;
    msg->compressiontype = 0x3;
    msg->unk = 0;
    msg->serial = house->serial_ext;
    msg->revision = ctBEu32(house->revision);
    msg->numtiles = ctBEu16(static_cast<u16>(pdesign->TotalSize()));

    msg->buffer->planecount = planes;
    buffer_len = 1;
    for( int i=0; i<planes; i++ )
    {
        planeheader = 0;
        data = pdesign->Compress(i, &ulen, &clen);
        if(data == NULL) //compression error
        {   
            return;
        }
        if(ulen == 0)
            clen = 0;
        planeheader |= ((mode << 4) << 24);
        planeheader |= ((i & 0xF) << 24);
        planeheader |= ((ulen & 0xFF) << 16);
        planeheader |= ((clen & 0xFF) << 8);
        planeheader |= (((ulen >> 4) & 0xF0) | ((clen >> 8) & 0xF)) ;
        u32* p_planeheader = reinterpret_cast<u32*>(&(packet[buffer_len+data_offset]));
        *p_planeheader = ctBEu32(planeheader);
        buffer_len += 4;
        memcpy(&(packet[buffer_len+data_offset]),data,clen);
        buffer_len += clen;
        delete[] data;
    }
    msg->msglen = ctBEu16( static_cast<u16>(buffer_len)+data_offset );
    msg->planebuffer_len = ctBEu16( static_cast<u16>(buffer_len) );
    
    ADDTOSENDQUEUE(client,&packet[0],cfBEu16(msg->msglen));
    stored_packet->swap(packet);
}

void CustomHousesSendFullToInRange(UHouse* house, int design, int range)
{
    unsigned short wxL, wyL, wxH, wyH;

    zone_convert_clip( house->x - range, house->y - range, house->realm, wxL, wyL );
    zone_convert_clip( house->x + range, house->y + range, house->realm, wxH, wyH );
    passert( wxL <= wxH );
    passert( wyL <= wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneCharacters& wchr = house->realm->zone[wx][wy].characters;

            for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
            {
                Character* chr = *itr;
				if(chr->has_active_client())
					CustomHousesSendFull(house, chr->client, design);
            }
        }
    }
}

void CustomHousesSendShort(UHouse* house, Client* client)
{
	PktOut_BF_Sub1D* msg = REQUESTSUBPACKET(PktOut_BF_Sub1D,PKTBI_BF_ID,PKTBI_BF::TYPE_CUSTOM_HOUSE_SHORT);
	msg->WriteFlipped(static_cast<u16>(13));
	msg->offset+=2;
	msg->Write(house->serial_ext);
	msg->WriteFlipped(house->revision);
    ADDTOSENDQUEUE(client,&msg->buffer, msg->offset);
	READDPACKET(msg);
}

void UHouse::SetCustom(bool _custom)
{
    if(custom == false && _custom == true)
        CustomHouseSetInitialState();

    custom = _custom;
}
void UHouse::CustomHouseSetInitialState()
{
    int ysize,xsize,xbase,ybase;
    const MultiDef& def = multidef();
    ysize = def.maxry - def.minry + 1; //+1 to include offset 0 in -3..3
    xsize = def.maxrx - def.minrx + 1; //+1 to include offset 0 in -3..3
    xbase = abs(def.minrx);
    ybase = abs(def.minry);
    CurrentDesign.Clear();
    CurrentDesign.InitDesign(ysize+1,xsize,xbase,ybase); //+1 for front steps outside multidef footprint
    WorkingDesign.Clear();
    WorkingDesign.InitDesign(ysize+1,xsize,xbase,ybase); //+1 for front steps outside multidef footprint
    BackupDesign.Clear();
    BackupDesign.InitDesign(ysize+1,xsize,xbase,ybase); //+1 for front steps outside multidef footprint

    CurrentDesign.AddMultiAtOffset(graphic,0,0,0);
    WorkingDesign = CurrentDesign;
    vector<u8> newvec;
    WorkingCompressed.swap(newvec);

    vector<u8> newvec2;
    CurrentCompressed.swap(newvec2);
}

