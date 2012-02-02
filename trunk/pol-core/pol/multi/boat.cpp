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
2011/11/12 Tomi:	  added extobj.tillerman, extobj.port_plank, extobj.starboard_plank and extobj.hold
2011/12/13 Tomi:      added support for new boats

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
#include "../network/packets.h"
#include "../network/clienttransmit.h"
#include "../core.h"
#include "../extobj.h"
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

    struct ComponentShape {
		unsigned int objtype;
        unsigned short graphic;
		unsigned short altgraphic;
        unsigned short xdelta;
        unsigned short ydelta;
		signed short zdelta;
		ComponentShape( const string& str, const string& altstr, unsigned char type );
        ComponentShape( const string& str, unsigned char type );
    };
    vector<ComponentShape> Componentshapes;

    bool objtype_is_component( unsigned int objtype ) const;
    BoatShape( ConfigElem& elem );
    BoatShape();
};

BoatShape::ComponentShape::ComponentShape( const string& str, unsigned char type )
{
	objtype = get_component_objtype( type );
    ISTRINGSTREAM is( str );
    string tmp;
    if (is >> tmp)
    {
        graphic = static_cast<unsigned short>(strtoul( tmp.c_str(), NULL, 0 ));
        if (graphic)
        {
            unsigned short xd, yd;
			signed short zd;
            if (is >> xd >> yd)
            {
                xdelta = xd;
                ydelta = yd;
				zdelta = 0;
				if (is >> zd)
					zdelta = zd;
                return;
            }
        }
    }

    cerr << "Boat component definition '" << str << "' is poorly formed." << endl;
    throw runtime_error( "Poorly formed boat.cfg component definition" );
}

BoatShape::ComponentShape::ComponentShape( const string& str, const string& altstr, unsigned char type )
{
	bool ok = false;
	objtype = get_component_objtype( type );
	ISTRINGSTREAM is( str );
    string tmp;
    if (is >> tmp)
    {
        graphic = static_cast<unsigned short>(strtoul( tmp.c_str(), NULL, 0 ));
        if (graphic)
        {
			unsigned short xd, yd;
			signed short zd;
            if (is >> xd >> yd)
            {
                xdelta = xd;
                ydelta = yd;
				zdelta = 0;
				if (is >> zd)
					zdelta = zd;
                ok = true;
            }
        }
    }

	ISTRINGSTREAM altis( altstr );
	string alttmp;
    if (ok && altis >> alttmp)
    {
        altgraphic = static_cast<unsigned short>(strtoul( alttmp.c_str(), NULL, 0 ));
		return;
    }
	else
		ok = false;

	if ( !ok )
	{
		cerr << "Boat component definition '" << str << "' is poorly formed." << endl;
		throw runtime_error( "Poorly formed boat.cfg component definition" );
	}
}


BoatShape::BoatShape()
{
}
BoatShape::BoatShape( ConfigElem& elem )
{
	string tmp_str;

	while( elem.remove_prop( "Tillerman", &tmp_str ) )
		Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_TILLERMAN ) );
	while( elem.remove_prop( "PortGangplankRetracted", &tmp_str ) )
		Componentshapes.push_back( ComponentShape( tmp_str, elem.remove_string( "PortGangplankExtended" ), COMPONENT_PORT_PLANK ) );
	while( elem.remove_prop( "StarboardGangplankRetracted", &tmp_str ) )
		Componentshapes.push_back( ComponentShape( tmp_str, elem.remove_string( "StarboardGangplankExtended" ), COMPONENT_STARBOARD_PLANK ) );
	while( elem.remove_prop( "Hold", &tmp_str ) )
		Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_HOLD ) );
	while( elem.remove_prop( "Rope", &tmp_str ) )
		Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_ROPE ) );
	while( elem.remove_prop( "Wheel", &tmp_str ) )
		Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_WHEEL ) );
	while( elem.remove_prop( "Hull", &tmp_str ) )
		Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_HULL ) );
	while( elem.remove_prop("Tiller", &tmp_str ) )
		Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_TILLER ) );
	while( elem.remove_prop( "Rudder", &tmp_str ) )
		Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_RUDDER ) );
	while( elem.remove_prop( "Sails", &tmp_str ) )
		Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_SAILS ) );
	while( elem.remove_prop( "Storage", &tmp_str ) )
		Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_STORAGE ) );
	while( elem.remove_prop( "Weaponslot", &tmp_str ) )
		Componentshapes.push_back( ComponentShape( tmp_str, COMPONENT_WEAPONSLOT ) );
}

bool BoatShape::objtype_is_component( unsigned int objtype ) const
{
	if(objtype == extobj.tillerman)
		return true;
	else if(objtype == extobj.port_plank)
		return true;
	else if(objtype == extobj.starboard_plank)
		return true;
	else if(objtype == extobj.hold)
		return true;
	else if(objtype == extobj.rope)
		return true;
	else if(objtype == extobj.wheel)
		return true;
	else if(objtype == extobj.hull)
		return true;
	else if(objtype == extobj.tiller)
		return true;
	else if(objtype == extobj.rudder)
		return true;
	else if(objtype == extobj.sails)
		return true;
	else if(objtype == extobj.storage)
		return true;
	else if(objtype == extobj.weaponslot)
		return true;
	else
		return false;
}

unsigned int get_component_objtype( unsigned char type )
{
	switch(type)
	{
		case COMPONENT_TILLERMAN: return extobj.tillerman;
		case COMPONENT_PORT_PLANK: return extobj.port_plank;
		case COMPONENT_STARBOARD_PLANK: return extobj.starboard_plank;
		case COMPONENT_HOLD: return extobj.hold;	
		case COMPONENT_ROPE: return extobj.rope;
		case COMPONENT_WHEEL: return extobj.wheel;
		case COMPONENT_HULL: return extobj.hull;
		case COMPONENT_TILLER: return extobj.tiller;
		case COMPONENT_RUDDER: return extobj.rudder;
		case COMPONENT_SAILS: return extobj.sails;
		case COMPONENT_STORAGE: return extobj.storage;
		case COMPONENT_WEAPONSLOT: return extobj.weaponslot;
		default: return 0;
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
        unsigned short multiid = elem.remove_ushort( "MultiID" );
        try {
            boatshapes[multiid] = new BoatShape(elem);
        }
        catch(exception&)
        {
            cerr << "Error occurred reading definition for boat 0x" << hex << multiid << dec << endl;
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

bool BoatShapeExists( u16 multiid )
{
    return boatshapes.count(multiid) != 0;
}

vector<Client*> boat_sent_to;

void send_boat_to_inrange( const UBoat* item, u16 oldx, u16 oldy )
{
	PktHelper::PacketOut<PktOut_1A> msg;
	msg->offset+=2;
	u16 graphic=item->multidef().multiid | 0x4000;
	msg->Write(item->serial_ext);
	msg->WriteFlipped(graphic);
	msg->WriteFlipped(item->x);
	msg->WriteFlipped(item->y);
	msg->Write(item->z);
	u16 len1A=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len1A);

	// Client >= 7.0.0.0 ( SA )
	PktHelper::PacketOut<PktOut_F3> msg2;
	msg2->WriteFlipped(static_cast<u16>(0x1));
	msg2->Write(static_cast<u8>(0x02));
	msg2->Write(item->serial_ext);
	msg2->WriteFlipped(item->multidef().multiid);
	msg2->offset++; // 0;
	msg2->WriteFlipped(static_cast<u16>(0x1)); //amount
	msg2->WriteFlipped(static_cast<u16>(0x1)); //amount2
	msg2->WriteFlipped(item->x);
	msg2->WriteFlipped(item->y);
	msg2->Write(item->z);
	msg2->offset++; // u8 facing
	msg2->WriteFlipped(item->color); // u16 color
	msg2->offset++; // u8 flags

	// Client >= 7.0.9.0 ( HSA )
	PktHelper::PacketOut<PktOut_F3> msg3;
	memcpy( &msg3->buffer, &msg2->buffer, sizeof msg3->buffer );
	msg3->offset=26; //unk short at the end

	PktHelper::PacketOut<PktOut_1D> msgremove;
	msgremove->Write(item->serial_ext);
    
	for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
	{
		Client* client = *itr;

		if (!client->ready)
			continue;
		if (inrange( client->chr, item ))
        {
            client->pause();
			if (client->ClientType & CLIENTTYPE_7090)
				msg3.Send(client);
			else if (client->ClientType & CLIENTTYPE_7000)
				msg2.Send(client);
			else
				msg.Send(client, len1A);
            boat_sent_to.push_back( client );
        }
		else if ((oldx!=USHRT_MAX) && (oldy!=USHRT_MAX) && //were inrange
			(client->chr->realm == item->realm) &&
			(inrange( client->chr->x, client->chr->y, oldx, oldy )))
		{
			msgremove.Send(client);
		}
    }
}

void unpause_paused()
{
    for( Clients::iterator itr = boat_sent_to.begin(), end=boat_sent_to.end(); itr != end; ++itr )
    {
        Client* client = *itr;
        client->restart();
    }
    boat_sent_to.clear();
}


UBoat::UBoat( const ItemDesc& descriptor ) : UMulti( descriptor )
{
    passert( boatshapes.count(multiid) != 0 );
	tillerman = NULL;
	hold = NULL;
	portplank = NULL;
	starboardplank = NULL;
}

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
        
        unsigned int gh = realm->encode_global_hull( ax, ay );
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
        
        unsigned int gh = realm->encode_global_hull( ax, ay );
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
            unsigned int gh = realm->encode_global_hull( ax, ay );
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
    for( vector<Item*>::iterator itr = Components.begin(), end = Components.end(); itr != end; ++itr )
    {
        Item* item = *itr;
		item->z += delta_z;
	}
}

void UBoat::on_color_changed()
{
    send_boat_to_inrange( this );
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
    for( vector<Item*>::iterator itr = Components.begin(), end = Components.end(); itr != end; ++itr )
    {
        Item* item = *itr;
		item->realm = realm;
	}
}

bool UBoat::deck_empty() const
{
    return travellers_.empty();
}

bool UBoat::hold_empty() const
{
    Item* it = this->hold;
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
bool UBoat::move_xy(unsigned short newx, unsigned short newy, int flags, Realm* oldrealm)
{
    bool result;
    
    unregself();
    
    if ( (flags & MOVEITEM_FORCELOCATION) || navigable(multidef(),newx, newy, z, realm))
    {
        BoatContext bc(*this);

        set_dirty();
        move_multi_in_world( x, y, newx, newy, this, oldrealm );

		u16 oldx = x;
		u16 oldy = y;
        x = newx;
        y = newy;

        // NOTE, send_boat_to_inrange pauses those it sends to.
        send_boat_to_inrange( this, oldx, oldy );
        move_components( oldrealm );
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

		u16 oldx = x;
		u16 oldy = y;
        x = newx;
        y = newy;

            // NOTE, send_boat_to_inrange pauses those it sends to.
        send_boat_to_inrange( this, oldx, oldy );
        move_components( realm );
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

inline unsigned short UBoat::multiid_ifturn( RELATIVE_DIR dir )
{
    return (multiid & ~3u) | ((multiid + dir) & 3);
}

const MultiDef& UBoat::multi_ifturn( RELATIVE_DIR dir )
{
    unsigned short multiid = multiid_ifturn(dir);
	passert( MultiDefByMultiIDExists(multiid) );
	return *MultiDefByMultiID(multiid);
}

UFACING UBoat::boat_facing() const
{
    return static_cast<UFACING>((multiid&3)*2);
}

const BoatShape& UBoat::boatshape() const
{
    passert( boatshapes.count(multiid) != 0 );
    return *boatshapes[ multiid ];
}


void UBoat::transform_components(const BoatShape& old_boatshape, Realm* oldrealm)
{
    const BoatShape& bshape = boatshape();
	vector<Item*>::iterator itr; 
	vector<Item*>::iterator end = Components.end();
	vector<BoatShape::ComponentShape>::const_iterator itr2;
	vector<BoatShape::ComponentShape>::const_iterator old_itr;
	vector<BoatShape::ComponentShape>::const_iterator end2 = bshape.Componentshapes.end();
	vector<BoatShape::ComponentShape>::const_iterator old_end = old_boatshape.Componentshapes.end();
	for (itr = Components.begin(), itr2 = bshape.Componentshapes.begin(), old_itr = old_boatshape.Componentshapes.begin(); 
		itr != end && itr2 != end2 && old_itr != old_end; 
		++itr, ++itr2, ++old_itr)
    {
        Item* item = *itr;
        if (item != NULL)
        {
            if (item->orphan())
            {
                continue;
            }
            item->set_dirty();
			if( item->objtype_ == extobj.port_plank && item->graphic == old_itr->altgraphic )
				item->graphic = itr2->altgraphic;
			else if( item->objtype_ == extobj.starboard_plank && item->graphic == old_itr->altgraphic )
				item->graphic = itr2->altgraphic;
			else
				item->graphic = itr2->graphic;
            move_boat_item( item, x + itr2->xdelta, y + itr2->ydelta, z + static_cast<s8>(itr2->zdelta), oldrealm );
        }
    }
}

void UBoat::move_components(Realm* oldrealm)
{
    const BoatShape& bshape = boatshape();
	vector<Item*>::iterator itr; 
	vector<Item*>::iterator end = Components.end();
	vector<BoatShape::ComponentShape>::const_iterator itr2;
	vector<BoatShape::ComponentShape>::const_iterator end2 = bshape.Componentshapes.end();
	for (itr = Components.begin(), itr2 = bshape.Componentshapes.begin(); 
		itr != end && itr2 != end2; 
		++itr, ++itr2)
    {
        Item* item = *itr;
        if (item != NULL)
        {
            if (item->orphan())
            {
                continue;
            }
            item->set_dirty();
            move_boat_item( item, x + itr2->xdelta, y + itr2->ydelta, z + static_cast<s8>(itr2->zdelta), oldrealm );
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
        multiid = multiid_ifturn(dir);

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

    if (portplank != NULL && !portplank->orphan())
	{
		plank = static_cast<UPlank*>(portplank);
        plank->setboat( this );
	}
    
    if (starboardplank != NULL && !starboardplank->orphan())
	{
		plank = static_cast<UPlank*>(starboardplank);
        plank->setboat( this );
	}
}

void UBoat::reread_components()
{
	for( vector<Item*>::const_iterator itr = Components.begin(), end = Components.end(); itr != end; ++itr )
    {
		Item* component = *itr;
        if (component == NULL) 
                continue;
		// check boat members here
		if ( component->objtype_ == extobj.tillerman && tillerman == NULL )
			tillerman = component;
		if ( component->objtype_ == extobj.port_plank && portplank == NULL )
			portplank = component;
		if ( component->objtype_ == extobj.starboard_plank && starboardplank == NULL )
			starboardplank = component;
		if ( component->objtype_ == extobj.hold && hold == NULL )
			hold = component;
	}
}

void UBoat::readProperties( ConfigElem& elem )
{
    base::readProperties( elem );

	multiid = elem.remove_ushort( "MultiID", this->multidef().multiid );

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
                if (bshape.objtype_is_component( item->objtype_ ))
                {
                    Components.push_back( item );
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
            if (bshape.objtype_is_component( item->objtype_ ))
            {
                Components.push_back( item );
            }
        }
    }
	reread_components();
    rescan_components();

    regself(); // do this after our x,y are known.
               // consider throwing if starting position isn't passable.

    start_script( "misc/boat", make_boatref(this) );
}

void UBoat::printProperties( ostream& os ) const
{
    base::printProperties( os );

	os << "\tMultiID\t" << multiid << pf_endl;

    BoatContext bc(*this);

    for( Travellers::const_iterator itr = travellers_.begin(), end = travellers_.end(); itr != end; ++itr )
    {
        UObject* obj = (*itr).get();
        if (!obj->orphan() &&  on_ship(bc,obj))
        {
            os << "\tTraveller\t0x" << hex << obj->serial << dec << pf_endl;
        }
    }
    for( vector<Item*>::const_iterator itr = Components.begin(), end = Components.end(); itr != end; ++itr )
    {
        Item* item = *itr;
        if (item != NULL && !item->orphan())
        {
            os << "\tComponent\t0x" << hex << item->serial << dec << pf_endl;
        }
    }
}

BObjectImp* UBoat::scripted_create( const ItemDesc& descriptor, u16 x, u16 y, s8 z, Realm* realm, int flags )
{
    unsigned short multiid = descriptor.multiid;
	unsigned short multiid_offset = static_cast<unsigned short>((flags & CRMULTI_FACING_MASK) >> CRMULTI_FACING_SHIFT);
	unsigned char facing = static_cast<unsigned char>(multiid_offset * 2);
	multiid += multiid_offset;

    const MultiDef* md = MultiDefByMultiID( multiid );
    if (md == NULL)
    {
        return new BError( "Multi definition not found for Boat, objtype=" 
                           + hexint(descriptor.objtype) + ", multiid=" 
                           + hexint(multiid) );
    }
    if (!boatshapes.count( descriptor.multiid ))
    {
        return new BError( "No boatshape for Boat in boats.cfg, objtype="
                           + hexint(descriptor.objtype) + ", multiid=" 
                           + hexint(multiid) );
    }

    if (!navigable( *md, x, y, z, realm ))
    {
        return new BError( "Position indicated is impassable" );
    }

    UBoat* boat = new UBoat( descriptor );
    boat->multiid = multiid;
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
	for (vector<BoatShape::ComponentShape>::const_iterator itr = bshape.Componentshapes.begin(), end = bshape.Componentshapes.end(); itr != end; ++itr)
    {
		Item* component = Item::create( itr->objtype );
        if (component == NULL) 
                continue;
		// check boat members here
		if ( component->objtype_ == extobj.tillerman && tillerman == NULL )
			tillerman = component;
		if ( component->objtype_ == extobj.port_plank && portplank == NULL )
			portplank = component;
		if ( component->objtype_ == extobj.starboard_plank && starboardplank == NULL )
			starboardplank = component;
		if ( component->objtype_ == extobj.hold && hold == NULL )
			hold = component;

        component->graphic = itr->graphic;
        // component itemdesc entries generally have graphic=1, so they don't get their height set.
        component->height = tileheight( component->graphic );
        component->x = x + itr->xdelta;
        component->y = y + itr->ydelta;
		component->z = z + static_cast<s8>(itr->zdelta);
        component->disable_decay();
        component->movable(false);
		component->realm = realm;
        add_item_to_world( component );
        update_item_to_inrange( component );
        Components.push_back( component );
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

BObjectImp* UBoat::component_list( unsigned char type ) const
{
	ObjArray* arr = new ObjArray;
	for( vector<Item*>::const_iterator itr = Components.begin(), end = Components.end(); itr != end; ++itr )
	{
		Item* item = *itr;
		if( item != NULL && !item->orphan() )
		{
			if( type == COMPONENT_ALL )
			{
				arr->addElement( make_itemref( item ) );
			}
			else
			{
				if( item->objtype_ == get_component_objtype( type ) )
					arr->addElement( make_itemref( item ) );
			}
		}
	}
	return arr;
}

void UBoat::destroy_components()
{
    for( vector<Item*>::iterator itr = Components.begin(), end = Components.end(); itr != end; ++itr )
    {
        Item* item = *itr;
        if (item != NULL && !item->orphan())
        {
            ::destroy_item( item );
        }
    }
	Components.clear();
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

