/*
History
=======
2005/04/02 Shinigami: move_offline_mobiles - added realm param
2009/09/03 MuadDib:	  Changes for account related source file relocation
                      Changes for multi related source file relocation

Notes
=======

*/


#ifndef BOAT_H
#define BOAT_H

#include "multi.h"
#include "../poltype.h"
#include "../reftypes.h"
#include <climits>

namespace Pol {
  namespace Bscript {
	class BObjectImp;
  }
  namespace Plib {
    class Realm;
  }
  namespace Module {
    class EUBoatRefObjImp;
  }
  namespace Multi {
	struct BoatShape;
	class MultiDef;
	

	class UBoat : public UMulti
	{
	  typedef UMulti base;

	  class BoatContext
	  {
		const MultiDef& mdef;
		unsigned short x;
		unsigned short y;

		explicit BoatContext( const UBoat& ub ) : mdef( ub.multidef() ), x( ub.x ), y( ub.y ) {};
		friend class UBoat;
		BoatContext & operator=( const BoatContext & ) { return *this; }
	  };

	public:
	  virtual UBoat* as_boat();

	  bool move( Core::UFACING dir );
	  bool move_xy( unsigned short x, unsigned short y, int flags, Plib::Realm* oldrealm );

      enum RELATIVE_DIR { NO_TURN, RIGHT, AROUND, LEFT };
	  bool turn( RELATIVE_DIR dir );

	  virtual void register_object( Core::UObject* obj );
	  Core::UFACING boat_facing() const;

	  void cleanup_deck();
	  bool hold_empty() const;
	  bool deck_empty() const;
	  bool has_offline_mobiles() const;
	  void remove_orphans();
	  void destroy_components();
	  void regself();
	  void unregself();

	  static Bscript::BObjectImp* scripted_create( const Items::ItemDesc& descriptor, u16 x, u16 y, s8 z, Plib::Realm* realm, int flags );

	  virtual Bscript::BObjectImp* make_ref();
	  static bool navigable( const MultiDef&, unsigned short x, unsigned short y, short z, Plib::Realm* realm );
	  void realm_changed();
	  void adjust_traveller_z( s8 delta_z );

	  virtual void on_color_changed();


	  // Should these be arrays instead to support customizable boats with certain component choises ?
	  Items::Item* tillerman;
	  Items::Item* portplank;
	  Items::Item* starboardplank;
	  Items::Item* hold;

	protected:
	  void move_travellers( enum Core::UFACING facing, const BoatContext& oldlocation, unsigned short x = USHRT_MAX, unsigned short y = USHRT_MAX, Plib::Realm* oldrealm = NULL );
	  void turn_travellers( RELATIVE_DIR dir, const BoatContext& oldlocation );
	  void turn_traveller_coords( Mobile::Character* chr, RELATIVE_DIR dir );
	  static bool on_ship( const BoatContext& bc, const Core::UObject* obj );
	  void move_offline_mobiles( Core::xcoord x, Core::ycoord y, Core::zcoord z, Plib::Realm* realm );
	  const MultiDef& multi_ifturn( RELATIVE_DIR dir );
	  unsigned short multiid_ifturn( RELATIVE_DIR dir );


	  void do_tellmoves();
	  const BoatShape& boatshape() const;
	  void rescan_components();
	  void reread_components();
	  void transform_components( const BoatShape& old_boatshape, Plib::Realm* oldrealm );
	  void move_components( Plib::Realm* oldrealm );

	  explicit UBoat( const Items::ItemDesc& descriptor );
	  virtual void readProperties( Clib::ConfigElem& elem );
	  virtual void printProperties( Clib::StreamWriter& sw ) const;
	  friend class UMulti;

	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const; ///id test
	  virtual Bscript::BObjectImp* script_method( const char *methodname, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex );
	  virtual bool script_isa( unsigned isatype ) const;
	  Bscript::BObjectImp* items_list() const;
	  Bscript::BObjectImp* mobiles_list() const;
	  Bscript::BObjectImp* component_list( unsigned char type ) const;

	  friend class Module::EUBoatRefObjImp;
	private:
	  void create_components();
	  typedef Core::UObjectRef Traveller;
	  typedef std::vector< Traveller > Travellers;
	  Travellers travellers_;

	  vector<Items::Item*> Components;
	};

	enum BOAT_COMPONENT
	{
	  COMPONENT_TILLERMAN = 0,
	  COMPONENT_PORT_PLANK = 1,
	  COMPONENT_STARBOARD_PLANK = 2,
	  COMPONENT_HOLD = 3,
	  COMPONENT_ROPE = 4,
	  COMPONENT_WHEEL = 5,
	  COMPONENT_HULL = 6,
	  COMPONENT_TILLER = 7,
	  COMPONENT_RUDDER = 8,
	  COMPONENT_SAILS = 9,
	  COMPONENT_STORAGE = 10,
	  COMPONENT_WEAPONSLOT = 11,
	  COMPONENT_ALL = 12
	};

	Bscript::BObjectImp* destroy_boat( UBoat* boat );
	void send_boat_to_inrange( const UBoat* item, u16 oldx = USHRT_MAX, u16 oldy = USHRT_MAX );
	unsigned int get_component_objtype( unsigned char type );
	bool BoatShapeExists( u16 multiid );
	void clean_boatshapes();
  }
}
#endif
