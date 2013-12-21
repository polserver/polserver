/*
History
=======
2005/06/06 Shinigami: added readobjects template - to get a list of statics
2009/09/03 MuadDib:   Relocation of multi related cpp/h

Notes
=======

*/


#ifndef MULTI_H
#define MULTI_H

#include "multidef.h"

// also consider: multimap<unsigned int, unsigned int>
// unsigned int is (x << 16) | y
// unsigned int is z << 16 | objtype
// hell, even multimap<unsigned short,unsigned int>
// unsigned short is (x<<8)|y
// (relative signed x/y, -128 to +127
// unsigned int is z << 16 | objtype

#include "../item/item.h"
namespace Pol {
  namespace Bscript {
    class BObjectImp;
    class BStruct;
  }
  namespace Plib {
    class Realm;
  }
  namespace Multi {
	class MultiDef;
	
	const int CRMULTI_IGNORE_MULTIS = 0x0001;
	const int CRMULTI_IGNORE_OBJECTS = 0x0002;
	const int CRMULTI_IGNORE_FLATNESS = 0x0004;
	const int CRMULTI_FACING_NORTH = 0x0000;
	const int CRMULTI_FACING_EAST = 0x0100;
	const int CRMULTI_FACING_SOUTH = 0x0200;
	const int CRMULTI_FACING_WEST = 0x0300;
	const int CRMULTI_FACING_MASK = 0x0300;
	const int CRMULTI_FACING_SHIFT = 8;

	class UMulti : public Items::Item
	{
	  typedef Items::Item base;
	public:

	  u16 multiid;

	  static UMulti* create( const Items::ItemDesc& descriptor, u32 serial = 0 );

	  static Bscript::BObjectImp* scripted_create( const Items::ItemDesc& descriptor, u16 x, u16 y, s8 z, Plib::Realm* realm, int flags );

	  virtual void double_click( Network::Client* client );
	  virtual void register_object( UObject* obj );
	  virtual void unregister_object( UObject* obj );
	  virtual bool script_isa( unsigned isatype ) const;

	  const MultiDef& multidef() const;
	  virtual class UBoat* as_boat();
	  virtual class UHouse* as_house();
	  virtual Bscript::BObjectImp* make_ref();
	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const;

	  Bscript::BStruct* footprint() const;
	  virtual bool readshapes( Plib::MapShapeList& vec, s16 rx, s16 ry, s16 zbase ) { return false; };
	  virtual bool readobjects( Core::StaticList& vec, s16 rx, s16 ry, s16 zbase ) { return false; };
	protected:
	  explicit UMulti( const Items::ItemDesc& itemdesc );
	  virtual ~UMulti();

	  virtual const char* classname() const;
	  friend class ref_ptr<UMulti>;

	private:

	  //virtual void destroy(void);
	};
  }
  namespace Core {
    void send_multi( Network::Client* client, const Multi::UMulti* multi );
    void send_multi_to_inrange( const Multi::UMulti* multi );
  }
}
#endif
