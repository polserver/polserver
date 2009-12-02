/*
History
=======
2005/04/02 Shinigami: move_offline_mobiles - added realm param
2005/08/22 Shinigami: do_tellmoves - bugfix - sometimes we've destroyed objects because of control scripts
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/08/25 Shinigami: STLport-5.2.1 fix: <cassert> removed
                      STLport-5.2.1 fix: boat_components changed little bit
2009/09/03 MuadDib:	  Changes for account related source file relocation
					  Changes for multi related source file relocation
2009/12/02 Turley:    added 0xf3 packet - Tomi

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#include <algorithm>
#include <set>

#include "../../bscript/berror.h"

#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/endian.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"

#include "../uconst.h"
#include "../../plib/realm.h"

#include "boat.h"
#include "boatcomp.h"
#include "../mobile/charactr.h"
#include "../network/client.h"
#include "../core.h"
#include "../fnsearch.h"
#include "../item/itemdesc.h"
#include "../mdelta.h"
#include "../mkscrobj.h"
#include "multi.h"
#include "multidef.h"
#include "../objtype.h"
#include "../pktout.h"
#include "../realms.h"
#include "../scrsched.h"
#include "../tiles.h"
#include "../ufunc.h"
#include "../uofile.h"
#include "../ustruct.h"
#include "../uvars.h"
#include "../uworld.h"

#include "../objecthash.h"


//#define DEBUG_BOATS

struct BoatShape {
    enum BOATSHAPE_COMPONENT {
        BOATSHAPE_COMPONENT_TILLERMAN = 0,
        BOATSHAPE_COMPONENT_PORT_PLANK_EXTENDED = 1,
        BOATSHAPE_COMPONENT_PORT_PLANK_RETRACTED = 2,
        BOATSHAPE_COMPONENT_STARBOARD_PLANK_EXTENDED = 3,
        BOATSHAPE_COMPONENT_STARBOARD_PLANK_RETRACTED = 4,
        BOATSHAPE_COMPONENT_HOLD = 5,
        BOATSHAPE_COMPONENT__COUNT = 6
    };

    struct ComponentShape {
        unsigned short graphic;
        signed char xdelta;
        signed char ydelta;
        ComponentShape( const string& str );
        ComponentShape();
    };
    ComponentShape componentshapes[ BOATSHAPE_COMPONENT__COUNT ];

    bool objtype_is_component( unsigned short objtype, int* index ) const;
    BoatShape( ConfigElem& elem );
    BoatShape();
};

struct Component {
    const char* name;
    bool create;
    UBoat::BOAT_COMPONENT component_idx_;
    unsigned short objtype_;
} boat_components[BoatShape::BOATSHAPE_COMPONENT__COUNT] = {
    { "Tillerman",                      true ,  UBoat::COMPONENT_TILLERMAN, EXTOBJ_TILLERMAN },
    { "PortGangplankExtended",          false,  UBoat::COMPONENT_PORT_PLANK, EXTOBJ_PORT_PLANK },
    { "PortGangplankRetracted",         true,   UBoat::COMPONENT_PORT_PLANK, EXTOBJ_PORT_PLANK },
    { "StarboardGangplankExtended",     false,  UBoat::COMPONENT_STARBOARD_PLANK, EXTOBJ_STARBOARD_PLANK },
    { "StarboardGangplankRetracted",    true,   UBoat::COMPONENT_STARBOARD_PLANK, EXTOBJ_STARBOARD_PLANK },
    { "Hold",                           true,   UBoat::COMPONENT_HOLD, EXTOBJ_HOLD }
};


BoatShape::ComponentShape::ComponentShape() : graphic(0), xdelta(0),ydelta(0)   //, create(false)
{
}
BoatShape::ComponentShape::ComponentShape( const string& str )
{
    ISTRINGSTREAM is( str );
    string tmp;
     // gcc screws this up
    if (is >> tmp)
    {
        graphic = static_cast<unsigned short>(strtoul( tmp.c_str(), NULL, 0 ));
        if (graphic)
        {
            int xd, yd;
            if (is >> xd >> yd)
            {
                xdelta = static_cast<signed char>(xd);
                ydelta = static_cast<signed char>(yd);
                return;
            }
        }

    }

    cerr << "Boat component definition '" << str << "' is poorly formed." << endl;
    throw runtime_error( "Poorly formed boat.cfg component definition" );
}

BoatShape::BoatShape()
{
}
BoatShape::BoatShape( ConfigElem& elem )
{
    for( int i = 0; i < BOATSHAPE_COMPONENT__COUNT; ++i )
    {
        try {
            componentshapes[i] = ComponentShape( elem.remove_string( boat_components[i].name ) );
        } 
        catch ( exception& ex )
        {
            cerr << "Error occurred reading boat component " << boat_components[i].name << ":" << ex.what() << endl;
            throw;
        }
    }
}
bool BoatShape::objtype_is_component( unsigned short objtype, int* index ) const
{
    if (objtype >= EXTOBJ_TILLERMAN && objtype <= EXTOBJ_HOLD)
    {
        *index = objtype - EXTOBJ_TILLERMAN;
        return true;
    }
    else
    {
        return false;
    }
}

typedef map< u16 /* graphic */ , BoatShape* > BoatShapes;
BoatShapes boatshapes;

void read_boat_cfg(void)
{
    ConfigFile cf( "config/boats.cfg", "Boat" );
    ConfigElem elem;
    while (cf.read( elem ))
    {
        unsigned short graphic = elem.remove_ushort( "ObjType" );
        try {
            boatshapes[graphic] = new BoatShape(elem);
        }
        catch(exception&)
        {
            cerr << "Error occurred reading definition for boat 0x" << hex << graphic << dec << endl;
            throw;
        }
    }
}

void clean_boatshapes()
{
	BoatShapes::iterator iter = boatshapes.begin();
	for ( ; iter != boatshapes.end(); ++iter) {
		if (iter->second != NULL)
			delete iter->second;
		iter->second = NULL;
	}
	boatshapes.clear();
}

bool BoatShapeExists( u16 graphic )
{
    return boatshapes.count(graphic) != 0;
}

vector<Client*> boat_sent_to;

void send_boat_to_inrange( const UBoat* item )
{

	PKTOUT_1A_C msg;
	msg.msgtype = PKTOUT_1A_C_ID;
	msg.msglen = ctBEu16(sizeof msg);
	msg.serial = item->serial_ext;
	msg.graphic = item->graphic_ext;
	msg.x = ctBEu16( item->x );
	msg.y = ctBEu16( item->y );
	msg.z = item->z;

	// Client >= 7.0.0.0 ( SA )
	PKTOUT_F3 msg2;
	msg2.msgtype = PKTOUT_F3_ID;
	msg2.unknown = ctBEu16( 0x1 );
	msg2.datatype = 0x02;
	msg2.serial = item->serial_ext;
	msg2.graphic = ctBEu16( item->multidef().multiid );
	msg2.facing = 0x00;
	msg2.amount = ctBEu16( 0x1 );
	msg2.amount_2 = ctBEu16( 0x1 );
	msg2.x = ctBEu16( item->x );
	msg2.y = ctBEu16( item->y );
	msg2.z = item->z;
	msg2.layer = 0x00;
	msg2.color = 0x00;
	msg2.flags = 0x00;
    
    for( Clients::iterator itr = clients.begin(); itr != clients.end(); ++itr )
	{
		Client* client = *itr;

		if (!client->ready)
			continue;
		if (inrange( client->chr, item ))
        {
            client->pause();
			if (client->ClientType & CLIENTTYPE_7000)
				client->transmit( &msg2, sizeof msg2 );
			else
				client->transmit( &msg, sizeof msg );
            boat_sent_to.push_back( client );
        }
    }
}

void unpause_paused()
{
    for( Clients::iterator itr = boat_sent_to.begin(); itr != boat_sent_to.end(); ++itr )
    {
        Client* client = *itr;
        client->restart();
    }
    boat_sent_to.clear();
}


UBoat::UBoat( const ItemDesc& descriptor ) : UMulti( descriptor )
{
    passert( boatshapes.count(graphic) != 0 );

}

const char* UBoat::component_names[ COMPONENT__COUNT ] = 
{
    "tillerman",
    "portplank",
    "starboardplank",
    "hold"
};

UBoat* UBoat::as_boat()
{
    return this;
}

/* boats are a bit strange - they can move from side to side, forwards and
   backwards, without turning.
 */
void UBoat::regself()
{
    const MultiDef& md = multidef();
    for( MultiDef::HullList::const_iterator itr = md.hull.begin(), end = md.hull.end();
         itr != end;
         ++itr )
    {
        unsigned short ax = x + (*itr)->x;
        unsigned short ay = y + (*itr)->y;
        
        unsigned long gh = realm->encode_global_hull( ax, ay );
        realm->global_hulls.insert( gh );
    }
}

void UBoat::unregself()
{
    const MultiDef& md = multidef();
    for( MultiDef::HullList::const_iterator itr = md.hull.begin(), end = md.hull.end();
         itr != end;
         ++itr )
    {
        unsigned short ax = x + (*itr)->x;
        unsigned short ay = y + (*itr)->y;
        
        unsigned long gh = realm->encode_global_hull( ax, ay );
        realm->global_hulls.erase( gh );
    }
}

// navigable: Can the ship sit here?  ie is every point on the hull on water,and not blocked?
bool UBoat::navigable( const MultiDef& md, unsigned short x, unsigned short y, short z, Realm* realm) 
{

	if (int(x + md.minrx) < int(WORLD_MIN_X) || int(x + md.maxrx) > int(realm->width()) ||
        int(y + md.minry) < int(WORLD_MIN_Y) || int(y + md.maxry) > int(realm->height()))
    {
#ifdef DEBUG_BOATS
            cout << "Location " << x << "," << y << " impassable, location is off the map" << endl;
#endif
        return false;
    }

    /* Test the external hull to make sure it's on water */

    for( MultiDef::HullList::const_iterator itr = md.hull.begin(), end = md.hull.end();
         itr != end;
         ++itr )
    {
        unsigned short ax = x + (*itr)->x;
        unsigned short ay = y + (*itr)->y;
		short az = z + (*itr)->z;
#ifdef DEBUG_BOATS
		cout << "[" << ax << "," << ay << "]";
#endif
        /*
         * See if any other ship hulls occupy this space
         */
            unsigned long gh = realm->encode_global_hull( ax, ay );
            if (realm->global_hulls.count(gh)) // already a boat there
            {
#ifdef DEBUG_BOATS
					cout << "Location " << realm->name() << " " << ax << "," << ay << " already has a ship hull present" << endl;
#endif
                return false;
            }

        if (!realm->navigable( ax, ay, az, tile[(*itr)->objtype].height )) 
            return false;
        
    }

    return true;
}

// ugh, we've moved the ship already - but we're comparing using the character's coords
// which used to be on the ship.
// let's hope it's always illegal to stand on the edges. !!
bool UBoat::on_ship( const BoatContext& bc, const UObject* obj )
{
    if (IsItem(obj->serial))
    {
        const Item* item = static_cast<const Item*>(obj);
        if (item->container != NULL)
            return false;
    }
    short rx = obj->x - bc.x;
    short ry = obj->y - bc.y;

    return bc.mdef.body_contains( rx, ry );
}

void UBoat::move_travellers( UFACING facing, const BoatContext& oldlocation, unsigned short newx, unsigned short newy, Realm* oldrealm )
{
    bool any_orphans = false;

    for( Travellers::iterator itr = travellers_.begin(), end = travellers_.end();
         itr != end;
         ++itr )
    {
        UObject* obj = (*itr).get();
        
        // consider: occasional sweeps of all boats to reap orphans
        if (obj->orphan() || !on_ship(oldlocation,obj))
        {
            any_orphans = true;
            (*itr).clear();;
            continue;
        }

        obj->set_dirty();
        if (obj->ismobile())
        {
            Character* chr = static_cast<Character*>(obj);

            if (chr->logged_in)
            {
                chr->lastx = chr->x;
                chr->lasty = chr->y;

				if(newx != USHRT_MAX && newy != USHRT_MAX) //dave added 3/27/3, if move_xy was used, dont use facing
				{
					s16 dx,dy;
					dx = chr->x - oldlocation.x; //keeps relative distance from boat mast
					dy = chr->y - oldlocation.y;
					chr->x = newx + dx;
					chr->y = newy + dy;				
				}
				else
                {
					chr->x += move_delta[facing].xmove;
					chr->y += move_delta[facing].ymove;
				}

                MoveCharacterWorldPosition( chr->lastx, chr->lasty, chr->x, chr->y, chr, oldrealm );
                chr->position_changed();
	            if (chr->client != NULL)	
	            {		
					if(oldrealm != chr->realm)
					{
						send_new_subserver( chr->client );
						send_owncreate( chr->client, chr );
					}
		            chr->client->pause();
                    send_goxyz( chr->client, chr );
                     // lastx and lasty are set above so these two calls will work right.
	                 // FIXME these are also called, in this order, in MOVEMENT.CPP.
                     // should be consolidated.
                    send_objects_newly_inrange( chr->client );
		            chr->client->restart();
	            }
                chr->move_reason = Character::MULTIMOVE;
            }
            else    
            {       
                    // characters that are logged out move with the boat
                    // they aren't in the worldzones so this is real easy.
                chr->lastx = chr->x; // I think in this case setting last? isn't
                chr->lasty = chr->y; // necessary, but I'll do it anyway.

				if(newx != USHRT_MAX && newy != USHRT_MAX) //dave added 3/27/3, if move_xy was used, dont use facing
				{
					s16 dx,dy;
					dx = chr->x - oldlocation.x; //keeps relative distance from boat mast
					dy = chr->y - oldlocation.y;
					chr->x = newx + dx;
					chr->y = newy + dy;					
				}
				else
                {
					chr->x += move_delta[facing].xmove;
					chr->y += move_delta[facing].ymove;
				}
            }
            
        }
        else
        {
			Item* item = static_cast<Item*>(obj);
			if(newx != USHRT_MAX && newy != USHRT_MAX) //dave added 4/9/3, if move_xy was used, dont use facing
			{
				s16 dx,dy;
				dx = item->x - oldlocation.x; //keeps relative distance from boat mast
				dy = item->y - oldlocation.y;
				move_item( item, newx+dx, newy+dy, item->z, NULL );
			}
			else
			{
				move_item( item, facing );
			}
        }
    }
    
    if (any_orphans)
        remove_orphans();

    do_tellmoves();
}


void UBoat::turn_traveller_coords( Character* chr, RELATIVE_DIR dir )
{
    chr->lastx = chr->x;
    chr->lasty = chr->y;
    
    s16 xd = chr->x - x;
    s16 yd = chr->y - y;

    switch( dir )
    {
        case LEFT:
            chr->x = x + yd;
            chr->y = y - xd;
            chr->facing = static_cast<UFACING>((chr->facing+6)&7);
            break;
        case AROUND:
            chr->x = x - xd;
            chr->y = y - yd;
            chr->facing = static_cast<UFACING>((chr->facing+4)&7);
            break;
        case RIGHT:
            chr->x = x - yd;
            chr->y = y + xd;
            chr->facing = static_cast<UFACING>((chr->facing+2)&7);
            break;
        case NO_TURN:
            break;
    }
}

void UBoat::turn_travellers( RELATIVE_DIR dir, const BoatContext& oldlocation )
{
    bool any_orphans = false;

    for( Travellers::iterator itr = travellers_.begin(), end = travellers_.end();
         itr != end;
         ++itr )
    {
        UObject* obj = (*itr).get();
        
            // consider: occasional sweeps of all boats to reap orphans
        if (obj->orphan() || !on_ship(oldlocation,obj))
        {
            any_orphans = true;
            (*itr).clear();
            continue;
        }

        obj->set_dirty();
        if (obj->ismobile())
        {
            Character* chr = static_cast<Character*>(obj);
            if (chr->logged_in)
            {
                //send_remove_character_to_nearby( chr );


                turn_traveller_coords( chr, dir );


                MoveCharacterWorldPosition( chr->lastx, chr->lasty, chr->x, chr->y, chr, NULL );
                chr->position_changed();
	            if (chr->client != NULL)	
	            {
		            send_goxyz( chr->client, chr );
                     // lastx and lasty are set above so these two calls will work right.
	                 // FIXME these are also called, in this order, in MOVEMENT.CPP.
                     // should be consolidated.
                
                    send_objects_newly_inrange( chr->client );
	            }
                chr->move_reason = Character::MULTIMOVE;
                //chr->lastx = ~ (unsigned short) 0; // force tellmove() to send "owncreate" and not send deletes.
                //chr->lasty = ~ (unsigned short) 0;
            }
            else
            {
                turn_traveller_coords( chr, dir );
            }
        }
        else
        {
            Item* item = static_cast<Item*>(obj);
            s16 xd = item->x - x;
            s16 yd = item->y - y;
            u16 newx = item->x;
			u16 newy = item->y;
            switch( dir )
            {
                case NO_TURN:
                    newx = item->x;
                    newy = item->y;
                    break;
                case LEFT:
                    newx = x + yd;
                    newy = y - xd;
                    break;
                case AROUND:
                    newx = x - xd;
                    newy = y - yd;
                    break;
                case RIGHT:
                    newx = x - yd;
                    newy = y + xd;
                    break;
            }
            move_item( item, newx, newy, item->z, NULL );
        }
    }
    
    if (any_orphans)
        remove_orphans();

    do_tellmoves();
}

void UBoat::remove_orphans()
{
    bool any_orphan_travellers;
    do {
        any_orphan_travellers = false;

        for( Travellers::iterator itr = travellers_.begin(), end = travellers_.end();
             itr != end;
             ++itr )
        {
            UObject* obj = (*itr).get();
            if (obj == NULL)
            {
                set_dirty();
                travellers_.erase(itr);
                any_orphan_travellers = true;
                break;
            }
        }
    } while (any_orphan_travellers);
}

void UBoat::cleanup_deck()
{
    BoatContext bc(*this);

    for( Travellers::iterator itr = travellers_.begin(), end = travellers_.end();
         itr != end;
         ++itr )
    {
        UObject* obj = (*itr).get();
        
        if (obj->orphan() || !on_ship(bc,obj))
        {
            set_dirty();
            (*itr).clear();
        }
    }
    remove_orphans();
}

bool UBoat::has_offline_mobiles() const
{
    BoatContext bc(*this);

    for( Travellers::const_iterator itr = travellers_.begin(), end = travellers_.end();
         itr != end;
         ++itr )
    {
        UObject* obj = (*itr).get();
        
        if (!obj->orphan() && on_ship(bc,obj) && obj->ismobile())
        {
            Character* chr = static_cast<Character*>(obj);

            if (!chr->logged_in)
            {
                return true;
            }
        }
    }
    return false;
}

void UBoat::move_offline_mobiles( xcoord x, ycoord y, zcoord z, Realm* realm )
{
    BoatContext bc(*this);

    for( Travellers::iterator itr = travellers_.begin(), end = travellers_.end();
         itr != end;
         ++itr )
    {
        UObject* obj = (*itr).get();
        
        if (!obj->orphan() && on_ship(bc,obj) && obj->ismobile())
        {
            Character* chr = static_cast<Character*>(obj);

            if (!chr->logged_in)
            {
                chr->set_dirty();
                chr->x = x;
                chr->y = y;
                chr->z = static_cast<signed char>(z);
                chr->realm = realm;
                chr->realm_changed(); // not sure if neccessary...
                (*itr).clear();
            }
        }
    }
    remove_orphans();
}
       
void UBoat::adjust_traveller_z(s8 delta_z)
{
    for( Travellers::iterator itr = travellers_.begin(), end = travellers_.end();
         itr != end;
         ++itr )
    {
        UObject* obj = (*itr).get();
        
		obj->z += delta_z;
	}
    for( int i = 0; i < COMPONENT__COUNT; ++i )
    {
        Item* item = components_[i].get();
		item->z += delta_z;
	}
}

void UBoat::realm_changed()
{
    BoatContext bc(*this);

    for( Travellers::iterator itr = travellers_.begin(), end = travellers_.end();
         itr != end;
         ++itr )
    {
        UObject* obj = (*itr).get();
        
        if (!obj->orphan() && on_ship(bc,obj) && obj->ismobile())
        {
            Character* chr = static_cast<Character*>(obj);
			send_remove_character_to_nearby( chr );
			chr->realm = realm;
			chr->realm_changed();
		}
        if (!obj->orphan() && on_ship(bc,obj) && IsItem( obj->serial ))
        {
            Item* item = static_cast<Item*>(obj);
            item->realm = realm;
			if(item->isa(UObject::CLASS_CONTAINER))
			{
				UContainer* cont = static_cast<UContainer*>(item);
				cont->for_each_item(setrealm, (void*)realm);
			}
		}
	}
    for( int i = 0; i < COMPONENT__COUNT; ++i )
    {
        Item* item = components_[i].get();
		item->realm = realm;
	}
}

bool UBoat::deck_empty() const
{
    return travellers_.empty();
}

bool UBoat::hold_empty() const
{
    Item* it = components_[ COMPONENT_HOLD ].get();
    if (it == NULL)
    {
        return true;
    }
    UContainer* cont = static_cast<UContainer*>(it);
    return (cont->count() == 0);
}

void UBoat::do_tellmoves()
{
    // we only do tellmove here because tellmove also checks attacks.
    // this way, we move everyone, then check for attacks.

    for( Travellers::iterator itr = travellers_.begin(), end = travellers_.end();
         itr != end;
         ++itr )
    {
        UObject* obj = (*itr).get();

		if (obj != NULL) // sometimes we've destroyed objects because of control scripts
		{
			if (obj->ismobile())
			{
				Character* chr = static_cast<Character*>(obj);
				if(chr->isa(UObject::CLASS_NPC) || chr->has_active_client()) //dave 3/27/3, dont tell moves of offline PCs
					chr->tellmove();
			}
		}
    }
}

//dave 3/26/3 added
bool UBoat::move_xy(unsigned short newx, unsigned short newy, long flags, Realm* oldrealm)
{
    bool result;
    
    unregself();
    
    if ( (flags & MOVEITEM_FORCELOCATION) || navigable(multidef(),newx, newy, z, realm))
    {
        BoatContext bc(*this);

        set_dirty();
        move_multi_in_world( x, y, newx, newy, this, oldrealm );

        x = newx;
        y = newy;

        // NOTE, send_boat_to_inrange pauses those it sends to.
        send_boat_to_inrange( this ); // FIXME doesn't send deletes
        transform_components( boatshape(), oldrealm );
        move_travellers( FACING_N, bc, newx, newy, oldrealm ); //facing is ignored if params 3 & 4 are not USHRT_MAX
        unpause_paused();

        result = true;
    }
    else
    {
        result = false;
    }

    regself();

    return result;
}

bool UBoat::move( UFACING facing )
{
    bool result;
    
    unregself();

    unsigned short newx, newy;
    newx = x + move_delta[facing].xmove;
    newy = y + move_delta[facing].ymove;
    
    if (navigable(multidef(),newx, newy, z, realm))
    {
        BoatContext bc(*this);

        set_dirty();

        move_multi_in_world( x, y, newx, newy, this, realm );

        x = newx;
        y = newy;

            // NOTE, send_boat_to_inrange pauses those it sends to.
        send_boat_to_inrange( this ); // FIXME doesn't send deletes
        transform_components( boatshape(), realm );
        move_travellers( facing, bc, x, y, realm );
        unpause_paused();

        result = true;
    }
    else
    {
        result = false;
    }

    regself();

    return result;
}

inline unsigned short UBoat::graphic_ifturn( RELATIVE_DIR dir )
{
    return (graphic & ~3u) | ((graphic + dir) & 3);
}

const MultiDef& UBoat::multi_ifturn( RELATIVE_DIR dir )
{
    unsigned short graphic = graphic_ifturn(dir);
    passert( MultiDefByGraphicExists(graphic) );
    return *MultiDefByGraphic(graphic);
}

UFACING UBoat::boat_facing() const
{
    return static_cast<UFACING>((graphic&3)*2);
}

const BoatShape& UBoat::boatshape() const
{
    passert( boatshapes.count(graphic) != 0 );
    return *boatshapes[ graphic ];
}


void UBoat::transform_components(const BoatShape& old_boatshape, Realm* oldrealm)
{
    const BoatShape& bshape = boatshape();
    for( int i = 0; i < COMPONENT__COUNT; ++i )
    {
        Item* item = components_[i].get();
        if (item != NULL)
        {
            if (item->orphan())
            {
                components_[i].clear();
                continue;
            }
            
            BoatShape::BOATSHAPE_COMPONENT bdcomp = BoatShape::BOATSHAPE_COMPONENT_TILLERMAN;
            switch( i )
            {
                case COMPONENT_TILLERMAN:
                    bdcomp = BoatShape::BOATSHAPE_COMPONENT_TILLERMAN;
                    break;
                case COMPONENT_STARBOARD_PLANK:
                    if (item->graphic == old_boatshape.componentshapes[BoatShape::BOATSHAPE_COMPONENT_STARBOARD_PLANK_EXTENDED].graphic)
                        bdcomp = BoatShape::BOATSHAPE_COMPONENT_STARBOARD_PLANK_EXTENDED;
                    else if (item->graphic == old_boatshape.componentshapes[BoatShape::BOATSHAPE_COMPONENT_STARBOARD_PLANK_RETRACTED].graphic)
                        bdcomp = BoatShape::BOATSHAPE_COMPONENT_STARBOARD_PLANK_RETRACTED;
                    break;
                case COMPONENT_PORT_PLANK:
                    if (item->graphic == old_boatshape.componentshapes[BoatShape::BOATSHAPE_COMPONENT_PORT_PLANK_EXTENDED].graphic)
                        bdcomp = BoatShape::BOATSHAPE_COMPONENT_PORT_PLANK_EXTENDED;
                    else if (item->graphic == old_boatshape.componentshapes[BoatShape::BOATSHAPE_COMPONENT_PORT_PLANK_RETRACTED].graphic)
                        bdcomp = BoatShape::BOATSHAPE_COMPONENT_PORT_PLANK_RETRACTED;
                    break;
                case COMPONENT_HOLD:
                    bdcomp = BoatShape::BOATSHAPE_COMPONENT_HOLD;
                    break;
            }
            item->set_dirty();
            item->graphic = bshape.componentshapes[ bdcomp ].graphic;
            item->graphic_ext = ctBEu16(item->graphic);
            move_boat_item( item, 
                       x + bshape.componentshapes[bdcomp].xdelta,
                       y + bshape.componentshapes[bdcomp].ydelta,
                       z /* zdelta assumed to be 0 */,
					   oldrealm );
        }
    }
}

bool UBoat::turn( RELATIVE_DIR dir )
{
    bool result;
    unregself();

    const MultiDef& newmd = multi_ifturn(dir);

    if (navigable(newmd, x, y, z, realm))
    {
        BoatContext bc(*this);
        const BoatShape& old_boatshape = boatshape();

        set_dirty();
        graphic = graphic_ifturn(dir);
        graphic_ext = ctBEu16(graphic);

        send_boat_to_inrange(this); // pauses those it sends to
        transform_components( old_boatshape, NULL );
        turn_travellers( dir, bc );
        unpause_paused();
		facing = ((dir*2)+facing)&7;
        result = true;
    }
    else
    {
        result = false;
    }
    regself();
    return result;
}

void UBoat::register_object( UObject* obj )
{ 
    if (find( travellers_.begin(), travellers_.end(), obj ) == travellers_.end())
    {
        set_dirty();
        travellers_.push_back( Traveller(obj) );
    }
}

void UBoat::rescan_components()
{
    UPlank* plank;
    
    plank = static_cast<UPlank*>(components_[ COMPONENT_PORT_PLANK ].get());
    if (plank)
        plank->setboat( this );
    
    plank = static_cast<UPlank*>(components_[ COMPONENT_STARBOARD_PLANK ].get());
    if (plank)
        plank->setboat( this );
}

void UBoat::readProperties( ConfigElem& elem )
{
    base::readProperties( elem );

    BoatContext bc(*this);
    const BoatShape& bshape = boatshape();
    u32 tmp_serial;
    while (elem.remove_prop( "Traveller", &tmp_serial ))
    {
        if (IsItem( tmp_serial ))
        {
            Item* item = find_toplevel_item( tmp_serial );
            if (item != NULL)
            {
                int index;
                if (bshape.objtype_is_component( item->objtype_, &index ))
                {
                    components_[index].set( item );
                }
                else if (on_ship(bc,item))
                {
                    travellers_.push_back( Traveller(item) );
                }
            }
        }
        else
        {
            Character* chr = system_find_mobile( tmp_serial );

            if (chr != NULL)
            {
                if (on_ship(bc,chr))
                    travellers_.push_back( Traveller(chr) );
            }
        }
    }
    while (elem.remove_prop( "Component", &tmp_serial ))
    {
        Item* item = system_find_item( tmp_serial );
        if (item != NULL)
        {
            int index;
            if (bshape.objtype_is_component( item->objtype_, &index ))
            {
                components_[index].set( item );
            }
        }
    }
    rescan_components();

    regself(); // do this after our x,y are known.
               // consider throwing if starting position isn't passable.

    start_script( "misc/boat", make_boatref(this) );
}

void UBoat::printProperties( ostream& os ) const
{
    base::printProperties( os );

    BoatContext bc(*this);

    for( Travellers::const_iterator itr = travellers_.begin(), end = travellers_.end(); itr != end; ++itr )
    {
        UObject* obj = (*itr).get();
        if (!obj->orphan() &&  on_ship(bc,obj))
        {
            os << "\tTraveller\t0x" << hex << obj->serial << dec << pf_endl;
        }
    }
    for( int i = 0; i < COMPONENT__COUNT; ++i )
    {
        Item* item = components_[i].get();
        if (item != NULL && !item->orphan())
        {
            os << "\tComponent\t0x" << hex << item->serial << dec << pf_endl;
        }
    }
}

BObjectImp* UBoat::scripted_create( const ItemDesc& descriptor, u16 x, u16 y, s8 z, Realm* realm, long flags )
{
    unsigned short graphic = descriptor.graphic;
	unsigned short graphic_offset = static_cast<unsigned short>((flags & CRMULTI_FACING_MASK) >> CRMULTI_FACING_SHIFT);
	unsigned char facing = static_cast<unsigned char>(graphic_offset * 2);
	graphic += graphic_offset;

    const MultiDef* md = MultiDefByGraphic( graphic );
    if (md == NULL)
    {
        return new BError( "Multi definition not found for Boat, objtype=" 
                           + hexint(descriptor.objtype) + ", graphic=" 
                           + hexint(graphic) );
    }
    if (!boatshapes.count( descriptor.graphic ))
    {
        return new BError( "No boatshape for Boat in boats.cfg, objtype="
                           + hexint(descriptor.objtype) + ", graphic=" 
                           + hexint(graphic) );
    }

    if (!navigable( *md, x, y, z, realm ))
    {
        return new BError( "Position indicated is impassable" );
    }

    UBoat* boat = new UBoat( descriptor );
    boat->graphic = graphic;
    boat->graphic_ext = ctBEu16(graphic);
    boat->serial = GetNewItemSerialNumber();
    boat->serial_ext = ctBEu32( boat->serial );
    boat->x = x;
    boat->y = y;
    boat->z = z;
	boat->realm = realm;
	boat->facing = facing;
    add_multi_to_world(boat);
    send_boat_to_inrange( boat );
    boat->create_components();
    boat->rescan_components();
    unpause_paused();
    boat->regself();

	////hash
	objecthash.Insert(boat);
	////

    start_script( "misc/boat", make_boatref(boat) );
    return make_boatref(boat);
}

void UBoat::create_components()
{
    const BoatShape& bshape = boatshape();
    for( int i = 0; i < BoatShape::BOATSHAPE_COMPONENT__COUNT; ++i )
    {
        if (boat_components[i].create && bshape.componentshapes[i].graphic) 
        {
            Item* component = Item::create( boat_components[i].objtype_ );
            if (component == NULL) 
                continue;
            component->graphic = bshape.componentshapes[i].graphic;
            component->graphic_ext = ctBEu16( component->graphic );
            // component itemdesc entries generally have graphic=1, so they don't get their height set.
            component->height = tileheight( component->graphic );
            component->x = x + bshape.componentshapes[i].xdelta;
            component->y = y + bshape.componentshapes[i].ydelta;
            component->z = z;
            component->disable_decay();
            component->movable(false);
			component->realm = realm;
            add_item_to_world( component );
            update_item_to_inrange( component );
            components_[ boat_components[i].component_idx_ ].set( component );
        }
    }

}

BObjectImp* UBoat::items_list() const
{
    BoatContext bc(*this);
    ObjArray* arr = new ObjArray;

    for( Travellers::const_iterator itr = travellers_.begin(), end = travellers_.end(); itr != end; ++itr )
    {
        UObject* obj = (*itr).get();
        if (!obj->orphan() &&  on_ship(bc,obj) && IsItem( obj->serial ))
        {
            Item* item = static_cast<Item*>(obj);
            arr->addElement( make_itemref( item ) );
        }
    }
    return arr;
}

BObjectImp* UBoat::mobiles_list() const
{
    BoatContext bc(*this);
    ObjArray* arr = new ObjArray;
    for( Travellers::const_iterator itr = travellers_.begin(), end = travellers_.end(); itr != end; ++itr )
    {
        UObject* obj = (*itr).get();
        if (!obj->orphan() &&  on_ship(bc,obj) && obj->ismobile())
        {
            Character* chr = static_cast<Character*>(obj);
            if (chr->logged_in)
                arr->addElement( make_mobileref(chr) );
        }
    }
    return arr;
}

void UBoat::destroy_components()
{
    for( int i = 0; i < COMPONENT__COUNT; ++i )
    {
        Item* item = components_[i].get();
        if (item != NULL && !item->orphan())
        {
            ::destroy_item( item );
        }
        components_[i].clear();
    }
}

BObjectImp* destroy_boat( UBoat* boat )
{
    boat->cleanup_deck();

    if (!boat->hold_empty())
        return new BError( "There is cargo in the ship's hold" );
    if (boat->has_offline_mobiles())
        return new BError( "There are logged-out characters on the deck" );
    if (!boat->deck_empty())
        return new BError( "The deck is not empty" );
    
    boat->destroy_components();
    boat->unregself();

    ConstForEach( clients, send_remove_object_if_inrange,
                  static_cast<const Item*>(boat) );

    remove_multi_from_world( boat );
    boat->destroy();
    return new BLong(1);
}

