/*
History
=======
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/cfgelem.h"
#include "../clib/endian.h"
#include "../clib/stlutil.h"

#include "../bscript/bobject.h"
#include "../bscript/bstruct.h"
#include "../bscript/executor.h"
#include "../bscript/berror.h"
#include "../bscript/objmethods.h"

#include "../plib/realm.h"
#include "network/client.h"
#include "network/packets.h"
#include "fnsearch.h"
#include "item/itemdesc.h"
#include "pktboth.h"
#include "poltype.h"
#include "sockio.h"
#include "umap.h"
#include "ufunc.h"
#include "msghandl.h"


Map::Map( const MapDesc& mapdesc ) : 
    Item(mapdesc, CLASS_ITEM ),
    gumpwidth(0),
    gumpheight(0),
    editable( mapdesc.editable ),
    plotting( false ),
    pin_points( 0 ),
    xwest(0),
    xeast(0),
    ynorth(0),
    ysouth(0)
{
}

Map::~Map()
{
}

void Map::printProperties( std::ostream& os ) const
{
    base::printProperties( os );

    os << "\txwest\t"       << xwest << pf_endl;
    os << "\tynorth\t"      << ynorth << pf_endl;
    os << "\txeast\t"       << xeast << pf_endl;
    os << "\tysouth\t"      << ysouth << pf_endl;
    os << "\tgumpwidth\t"   << gumpwidth << pf_endl;
    os << "\tgumpheight\t"  << gumpheight << pf_endl;

	os << "\teditable\t"  << editable << pf_endl;
	
	printPinPoints(os);

	os << pf_endl;
	
}

void Map::printPinPoints( std::ostream& os ) const
{
/*
	ObjArray* arr = new ObjArray;
	PinPoints::const_iterator itr;

	for( itr = pin_points.begin(); itr != pin_points.end(); itr++ )
	{

		ObjArray* struc = new ObjArray;
		struc->addMember( "x", new BLong( itr->x) );
		struc->addMember( "y", new BLong( itr->y) );		
	
		arr->addElement( struc );
	}
	arr->packonto(os);
	delete arr;
*/
	PinPoints::const_iterator itr;
	int i=0;
	os << "\tNumPins " << pin_points.size() << pf_endl;

	for( itr = pin_points.begin(); itr != pin_points.end(); ++itr, ++i )
	{
		os << "\tPin" << i << " " << itr->x << "," << itr->y << pf_endl;
	}
}

void Map::readProperties( ConfigElem& elem )
{
    base::readProperties(elem);

    xwest       = elem.remove_ushort( "xwest", 0 );
    ynorth      = elem.remove_ushort( "ynorth", 0 );
    xeast       = elem.remove_ushort( "xeast", 0 );
    ysouth      = elem.remove_ushort( "ysouth", 0 );
    gumpwidth   = elem.remove_ushort( "gumpwidth", 0 );
    gumpheight  = elem.remove_ushort( "gumpheight", 0 );
	editable	= elem.remove_bool( "editable" , 0 );

	unsigned short numpins = elem.remove_ushort( "NumPins", 0 );
	string pinval;
	char search_string[6];
	int i,x,y;
	struct PinPoint pp;

	for( i=0; i<numpins; i++)
	{
		sprintf(search_string,"Pin%i",i);
		pinval = elem.remove_string( search_string );
		sscanf(pinval.c_str(),"%i,%i",&x,&y);
		
		pp.x = static_cast<unsigned short>(x);
		pp.y = static_cast<unsigned short>(y);
		
		pin_points.push_back(pp);
	}

}

void Map::builtin_on_use( Client* client )
{
    if (gumpwidth && gumpheight)
    {
		PktOut_90* msg90 = REQUESTPACKET(PktOut_90,PKTOUT_90_ID);
		msg90->Write(serial_ext);
		msg90->Write(static_cast<u8>(0x13));
		msg90->Write(static_cast<u8>(0x9d));
		msg90->WriteFlipped(xwest);
		msg90->WriteFlipped(ynorth);
		msg90->WriteFlipped(xeast);
		msg90->WriteFlipped(ysouth);
		msg90->WriteFlipped(gumpwidth);
		msg90->WriteFlipped(gumpheight);
		transmit( client, &msg90->buffer, msg90->offset );
		msg90->Test(msg90->offset);
		READDPACKET(msg90);

		PktOut_56* msg56 = REQUESTPACKET(PktOut_56,PKTBI_56_ID);
		msg56->Write(serial_ext);
		msg56->Write(static_cast<u8>(PKTBI_56::TYPE_REMOVE_ALL));
		msg56->offset+=5; // u8 pinidx,u16 pinx,piny

		transmit( client, &msg56->buffer, msg56->offset );
		msg56->Test(msg56->offset);

		//need to send each point to the client

		if(!pin_points.empty())
		{
			msg56->offset=5; //msgtype+serial_ext
			msg56->Write(static_cast<u8>(PKTBI_56::TYPE_ADD));
			// u8 pinidx still 0

			for( pin_points_itr itr = pin_points.begin(), end=pin_points.end(); itr != end; ++itr )
			{
				msg56->offset=7; //msgtype+serial_ext+type,pinidx
				msg56->WriteFlipped( worldXtoGumpX(itr->x) );
				msg56->WriteFlipped( worldYtoGumpY(itr->y) );
				transmit( client, &msg56->buffer, msg56->offset );
			}
		}
		msg56->Test(msg56->offset);
		READDPACKET(msg56);
    }
}

BObjectImp* Map::script_method_id( const int id, Executor& ex )
{
    BObjectImp* imp = base::script_method_id( id, ex );
    if (imp != NULL)
        return imp;

    switch(id)
    {
    case MTH_GETPINS:
        {
		    ObjArray* arr = new ObjArray;
		    for( PinPoints::iterator itr = pin_points.begin(); itr != pin_points.end(); ++itr )
		    {

			    BStruct* struc = new BStruct;
			    struc->addMember( "x", new BLong( itr->x) );
			    struc->addMember( "y", new BLong( itr->y) );		
    		
			    arr->addElement( struc );
		    }
		    return arr;
        }

    case MTH_INSERTPIN:
        {
            int idx;
	        unsigned short x, y;
            if (ex.getParam( 0, idx, pin_points.size() ) &&
                ex.getParam( 1, x ) &&
                ex.getParam( 2, y ))
            {
			    struct PinPoint pp;
			    pin_points_itr itr;

			    if(!realm->valid(x,y,0)) return new BError("Invalid Coordinates for Realm");
			    pp.x = x;
			    pp.y = y;

			    itr = pin_points.begin();
			    itr += idx;

                set_dirty();
			    pin_points.insert(itr,pp);

                return new BLong(1);
            }
            else
            {
                return new BError( "Invalid parameter type" );
            }
        }

    case MTH_APPENDPIN:
        {
            unsigned short x, y;
            if (ex.getParam( 0, x ) &&
                ex.getParam( 1, y ))
            {
			    struct PinPoint pp;
			    if(!realm->valid(x,y,0)) return new BError("Invalid Coordinates for Realm");
			    pp.x = x;
			    pp.y = y;
                set_dirty();
			    pin_points.push_back(pp);
                return new BLong(1);
            }
            else
            {
                return new BError( "Invalid parameter type" );
            }
        }

    case MTH_ERASEPIN:
        {
            int idx;
            if (ex.getParam( 0, idx, pin_points.size()-1 ))
            {
			    pin_points_itr itr;

			    itr = pin_points.begin();
			    itr += idx;

                set_dirty();
			    pin_points.erase(itr);
                return new BLong(1);
            }
            else
            {
                return new BError( "Index Out of Range" );
            }
        }

    case MTH_ISA:
        {
            int type;
            if (ex.getParam( 0, type ))
            {
			    return new BLong(script_isa(type));
            }
            else
            {
                return new BError( "Invalid Parameter" );
            }
        }

    default:
        return NULL;
    }
}


BObjectImp* Map::script_method( const char* methodname, Executor& ex )
{
	BObjectImp* imp = base::script_method(methodname, ex);
    if ( imp != NULL )
        return imp;

	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->script_method_id(objmethod->id, ex);
	else
		return NULL;
	/*
    BObjectImp* imp = base::script_method( methodname, ex );
    if (imp != NULL)
        return imp;

	if (stricmp( methodname, "getpins" ) == 0)
    {
		ObjArray* arr = new ObjArray;
		for( PinPoints::iterator itr = pin_points.begin(); itr != pin_points.end(); ++itr )
		{

			BStruct* struc = new BStruct;
			struc->addMember( "x", new BLong( itr->x) );
			struc->addMember( "y", new BLong( itr->y) );		
		
			arr->addElement( struc );
		}
		return arr;
    }
	else if(stricmp( methodname, "insertpin" ) == 0)
	{
		int idx, x, y;
        if (ex.getParam( 0, idx, pin_points.size() ) &&
            ex.getParam( 1, x ) &&
            ex.getParam( 2, y ))
        {
			struct PinPoint pp;
			pin_points_itr itr;

			if(!realm->valid(x,y,0)) return new BError("Invalid Coordinates for Realm");
			pp.x = (unsigned short) x;
			pp.y = (unsigned short) y;

			itr = pin_points.begin();
			itr += idx;

            set_dirty();
			pin_points.insert(itr,pp);

            return new BLong(1);
        }
        else
        {
            return new BError( "Invalid parameter type" );
        }
	}
	else if(stricmp( methodname, "appendpin" ) == 0)
	{
		int x, y;
        if (ex.getParam( 0, x ) &&
            ex.getParam( 1, y ))
        {
			struct PinPoint pp;
			if(!realm->valid(x,y,0)) return new BError("Invalid Coordinates for Realm");
			pp.x = (unsigned short) x;
			pp.y = (unsigned short) y;
            set_dirty();
			pin_points.push_back(pp);
            return new BLong(1);
        }
        else
        {
            return new BError( "Invalid parameter type" );
        }
	}
	else if(stricmp( methodname, "erasepin" ) == 0)
	{
		int idx;
        if (ex.getParam( 0, idx, pin_points.size()-1 ))
        {
			pin_points_itr itr;

			itr = pin_points.begin();
			itr += idx;

            set_dirty();
			pin_points.erase(itr);
            return new BLong(1);
        }
        else
        {
            return new BError( "Index Out of Range" );
        }
	}
	else if(stricmp( methodname, "isa" ) == 0)
	{
		int type;
        if (ex.getParam( 0, type ))
        {
			return new BLong(script_isa(type));
        }
        else
        {
            return new BError( "Invalid Parameter" );
        }
	}
    return NULL;
	*/
}

bool Map::msgCoordsInBounds( PKTBI_56* msg )
{
	u16 newx, newy;
	newx = cfBEu16(msg->pinx);
	newy = cfBEu16(msg->piny);

	if( ((newx+get_xwest()) > get_xeast()) || ((newy+get_ynorth()) > get_ysouth()) )
		return false;
	else
		return true;
}

u16 Map::gumpXtoWorldX(u16 gumpx)
{
	float world_xtiles_per_pixel = (float)(get_xeast() - get_xwest()) / (float)gumpwidth;
	u16 ret = (u16)(get_xwest() + (world_xtiles_per_pixel * gumpx));
	return ret;
}

u16 Map::gumpYtoWorldY(u16 gumpy)
{
	float world_ytiles_per_pixel = (float)(get_ysouth() - get_ynorth()) / (float)gumpheight;
	u16 ret = (u16)(get_ynorth() + (world_ytiles_per_pixel * gumpy));
	return ret;
}

u16 Map::worldXtoGumpX(u16 worldx)
{
	float world_xtiles_per_pixel = (float)(get_xeast() - get_xwest()) / (float)gumpwidth;
	u16 ret = (u16)((worldx - get_xwest()) / world_xtiles_per_pixel);
	return ret;
}

u16 Map::worldYtoGumpY(u16 worldy)
{
	float world_ytiles_per_pixel = (float)(get_ysouth() - get_ynorth()) / (float)gumpheight;
	u16 ret = (u16)((worldy - get_ynorth()) / world_ytiles_per_pixel);
	return ret;
}

//dave 12-20
Item* Map::clone() const
{
    Map* map = static_cast<Map*>(base::clone());

	map->gumpwidth = gumpwidth;
    map->gumpheight = gumpheight;
	map->editable = editable;
	map->plotting = plotting;	
	map->pin_points = pin_points;
	map->xwest = xwest;
    map->xeast = xeast;
    map->ynorth = ynorth;
    map->ysouth = ysouth;
    return map;
}

void handle_map_pin( Client* client, PKTBI_56* msg )
{
    // FIXME you really need to check that the item is in fact a map.
	// Can cause crash if someone is messing with their packets to script
	// pin movement on a non-map item.
	Map* my_map = (Map*) find_legal_item( client->chr, cfBEu32(msg->serial) );
	if(my_map == NULL)
		return;
	if(my_map->editable == false)
		return;

	static struct PinPoint pp;
	Map::pin_points_itr itr;

	switch( msg->type )
	{
		case PKTBI_56::TYPE_TOGGLE_EDIT:
		{
			//hmm msg->plotstate never seems to be 1 when type is 6
			my_map->plotting = my_map->plotting ? 0 : 1;
			PktOut_56* msg56 = REQUESTPACKET(PktOut_56,PKTBI_56_ID);
			msg56->Write(msg->serial);
			msg56->Write(static_cast<u8>(PKTBI_56::TYPE_TOGGLE_RESPONSE));
			msg56->Write(static_cast<u8>(my_map->plotting));
			msg56->Write(msg->pinx);
			msg56->Write(msg->piny);
			transmit( client, &msg56->buffer, msg56->offset );
			READDPACKET(msg56);
			break;
		}

		case PKTBI_56::TYPE_ADD:
			if(!(my_map->plotting))
				return;
			if(!my_map->msgCoordsInBounds(msg))
				return;
			
			pp.x = my_map->gumpXtoWorldX( cfBEu16(msg->pinx) );
			pp.y = my_map->gumpYtoWorldY( cfBEu16(msg->piny) );
			my_map->pin_points.push_back(pp);
			break;

		case PKTBI_56::TYPE_INSERT:
			if(!(my_map->plotting))
				return;
			if(msg->pinidx >= my_map->pin_points.size()) //pinidx out of range
				return;
			if(!my_map->msgCoordsInBounds(msg))
				return;

			itr = my_map->pin_points.begin();
			itr += msg->pinidx;
			
			pp.x = my_map->gumpXtoWorldX( cfBEu16(msg->pinx) );
			pp.y = my_map->gumpYtoWorldY( cfBEu16(msg->piny) );

			my_map->pin_points.insert(itr,pp);
			break;

		case PKTBI_56::TYPE_CHANGE:
			if(!(my_map->plotting))
				return;
			if(msg->pinidx >= my_map->pin_points.size()) //pinidx out of range
				return;
			if(!my_map->msgCoordsInBounds(msg))
				return;
			
			itr = my_map->pin_points.begin();
			itr += msg->pinidx;

			itr->x = my_map->gumpXtoWorldX( cfBEu16(msg->pinx) );
			itr->y = my_map->gumpYtoWorldY( cfBEu16(msg->piny) );

			break;

		case PKTBI_56::TYPE_REMOVE:
			if(!(my_map->plotting))
				return;
			if(msg->pinidx >= my_map->pin_points.size()) //pinidx out of range
				return;

			itr = my_map->pin_points.begin();
			itr += msg->pinidx;

			my_map->pin_points.erase(itr);

			break;

		case PKTBI_56::TYPE_REMOVE_ALL:
			if(!(my_map->plotting))
				return;

			my_map->pin_points.clear();

			break;

	}


}
MESSAGE_HANDLER( PKTBI_56, handle_map_pin );
