/*
History
=======


Notes
=======

*/

#ifndef UMAP_H
#define UMAP_H

#include "item/item.h"
#include "pktboth.h"

class BObjectImp;

struct PinPoint
{
    unsigned short x;
    unsigned short y;
};

class Map : public Item
{
    typedef Item base;
public:
    u16 gumpwidth;
    u16 gumpheight;
	bool editable;
	bool plotting;	
	typedef std::vector<PinPoint> PinPoints;
	PinPoints pin_points;
	typedef PinPoints::iterator pin_points_itr;

	inline u16 get_xwest(){return xwest;};
	inline u16 get_xeast(){return xeast;};
	inline u16 get_ynorth(){return ynorth;};
	inline u16 get_ysouth(){return ysouth;};

	virtual bool msgCoordsInBounds( PKTBI_56* msg );
	virtual u16 gumpXtoWorldX(u16 gumpx);
	virtual u16 gumpYtoWorldY(u16 gumpy);
	virtual u16 worldXtoGumpX(u16 worldx);
	virtual u16 worldYtoGumpY(u16 worldy);

	virtual Item* clone() const; //dave 12-20
protected:
    Map( const MapDesc& mapdesc );
    virtual ~Map();
    
    friend Item* Item::create( const ItemDesc& itemdesc, u32 serial );

    virtual void builtin_on_use( Client* client );
	virtual BObjectImp* script_method( const char* methodname, Executor& ex );
    virtual BObjectImp* script_method_id( const int id, Executor& ex );
    virtual BObjectImp* get_script_member( const char *membername ) const;
    virtual BObjectImp* get_script_member_id( const int id ) const; ///id test
    //virtual BObjectImp* set_script_member( const char *membername, const std::string& value );
    virtual BObjectImp* set_script_member( const char *membername, long value );
    virtual BObjectImp* set_script_member_id( const int id , long value ); //id test
    //virtual BObjectImp* set_script_member_double( const char *membername, double value );
    virtual bool script_isa( unsigned isatype ) const;
    virtual void printProperties( std::ostream& os ) const;
	virtual void printPinPoints( std::ostream& os ) const;
    virtual void readProperties( ConfigElem& elem );


private:
    u16 xwest;
    u16 xeast;
    u16 ynorth;
    u16 ysouth;

private: // not implemented
    Map( const Map& map );
    Map& operator=( const Map& map );
};

#endif

