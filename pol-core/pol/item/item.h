/*
History
=======
2008/12/17 MuadDub:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
2009/08/06 MuadDib:   Added gotten_by for character ref.
2009/08/06 MuadDib:   Added gotten_by code for items.

Notes
=======

*/

#ifndef ITEM_H
#define ITEM_H

#include "../uobject.h"


#ifndef BSCRIPT_BOBJECT_H
#include "../../bscript/bobject.h"
#endif

#include "../layers.h"
#include "../globals/settings.h"

namespace Pol {
  namespace Core {
	struct USTRUCT_TILE;
	class UContainer;
    std::string format_description( unsigned int polflags, const std::string& descdef, unsigned short amount, const std::string suffix );
  }
  namespace Mobile {
	class Character;
  }
  namespace Multi {
	class UMulti;
  }
  namespace Network {
	class Client;
  }
  namespace Items {
	class ItemDesc;

	// NOTE: All derived classes should have protected constructors, 
	//			and make Item a friend.
	class Item : public Core::UObject
	{
	  typedef Core::UObject base;
	public:
	  bool stackable() const;

	  virtual ~Item();
      virtual size_t estimatedSize( ) const POL_OVERRIDE;

	  virtual void double_click( Network::Client* client );
	  virtual void builtin_on_use( Network::Client* client );
	  virtual void walk_on( Mobile::Character* chr );
	  virtual u8 los_height() const POL_OVERRIDE;

	  const ItemDesc& itemdesc() const;

	  virtual u16 get_senditem_amount() const;
	  u16 getamount() const;
	  void setamount( u16 amount );
	  void subamount( u16 amount_subtract );

	  bool movable() const;
	  void movable( bool newvalue );
	  void on_movable_changed();

	  bool default_movable() const;
	  bool default_invisible() const;

	  bool inuse() const;
	  void inuse( bool newvalue );

	  bool is_gotten() const;
	  void set_gotten( Mobile::Character* by_char );
      Mobile::Character* get_gotten() const;

	  bool invisible() const;
	  void invisible( bool newvalue );
	  void on_invisible_changed();

	  virtual bool saveonexit() const POL_OVERRIDE;
	  virtual void saveonexit( bool newvalue ) POL_OVERRIDE;

	  void set_decay_after( unsigned int seconds );
	  bool should_decay( unsigned int gameclock ) const;
	  void restart_decay_timer();
	  void disable_decay();
      bool can_decay() const;

	  bool setlayer( unsigned char layer );
	  virtual bool setgraphic( u16 newobjtype ) POL_OVERRIDE;
	  virtual bool setcolor( u16 newcolor ) POL_OVERRIDE;
	  virtual void on_color_changed() POL_OVERRIDE;
	  virtual void spill_contents( Multi::UMulti* supporting_multi );

	  virtual void setfacing( u8 newfacing ) POL_OVERRIDE;
	  virtual void on_facing_changed() POL_OVERRIDE;

	  std::string description() const;
	  std::string merchant_description() const;
      

      std::string get_use_script_name() const;
	  u32 sellprice() const;
      void sellprice(u32);
	  u32 buyprice() const;
      void buyprice(u32);
	  bool getbuyprice( u32& buyprice ) const;

	  bool newbie() const;
	  bool default_newbie() const;
	  void newbie( bool newvalue );

	  u8 slot_index() const;
	  bool slot_index( u8 newvalue );
      void reset_slot();

	  virtual unsigned int item_count() const;
	  unsigned int weight_of( unsigned short amount ) const;
	  virtual unsigned int weight() const POL_OVERRIDE;

	  virtual std::string name() const POL_OVERRIDE;

      std::string name_suffix() const;
      void name_suffix(const std::string& name);

	  virtual UObject* owner() POL_OVERRIDE;
	  virtual const UObject* owner() const POL_OVERRIDE;
	  virtual UObject* toplevel_owner() POL_OVERRIDE;
	  virtual const UObject* toplevel_owner() const POL_OVERRIDE;

	  bool can_add_to_self( unsigned short amount ) const;
	  bool can_add_to_self( const Item& item ) const;
	  bool has_only_default_cprops( const ItemDesc* compare = NULL ) const;
	  void add_to_self( Item*& item ); // deletes the item passed

#ifdef PERGON
	  void ct_merge_stacks_pergon( Item*& item_sub ); // Pergon: Re-Calculate Property CreateTime after Merging of two Stacks
	   void ct_merge_stacks_pergon( u16 amount_sub );  // Pergon: Re-Calculate Property CreateTime after Adding Items to a Stack
#endif

	  bool amount_to_remove_is_partial( u16 amount_to_remove ) const;

	  // NOTE: this_item_amount is the amount that this item
	  //	  will have.  A new item will be created, whos amount
	  //		is the remainder. This is the returned item.
	  Item* slice_stacked_item( u16 this_item_new_amount );

	  Item* remove_part_of_stack( u16 amount_to_remove );

	  void set_use_script( const std::string& scriptname );
	  void extricate();

	  bool has_equip_script() const;
	  Bscript::BObjectImp* run_equip_script( Mobile::Character* chr, bool startup );
	  bool check_equip_script( Mobile::Character* chr, bool startup );
	  Bscript::BObjectImp* run_unequip_script( Mobile::Character* who );
	  bool check_unequip_script();

	  bool check_test_scripts( Mobile::Character* chr, const std::string& script_ecl, bool startup );
	  bool check_equiptest_scripts( Mobile::Character* chr, bool startup = false );
	  bool check_unequiptest_scripts( Mobile::Character* chr );
	  bool check_unequiptest_scripts();

	  //DAVE added this 11/17, Shortcut function to get a pointer to the owner character
	  Mobile::Character* GetCharacterOwner( );
	  // I'm using the named constructor idiom for creation,
	  // so that the right kind of object gets created
	  // and so that object serial numbers get generated.
	  // If no serial passed, one will be generated.
	  static Item* create( u32 objtype, u32 serial = 0 );
	  static Item* create( const ItemDesc& itemdesc, u32 serial = 0 );
	  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;
	  virtual Item* clone() const;
	  //virtual class BObjectImp* script_member( const char *membername );
	  virtual Bscript::BObjectImp* make_ref( ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE; ///id test
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string& value ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, int value ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, const std::string& value ) POL_OVERRIDE;///id test
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ) POL_OVERRIDE;///id test
	  virtual Bscript::BObjectImp* set_script_member_double( const char *membername, double value ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* set_script_member_id_double( const int id, double value ) POL_OVERRIDE; //id test
	  virtual Bscript::BObjectImp* script_method( const char *methodname, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex ) POL_OVERRIDE;

	  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;
	  virtual const char* target_tag() const POL_OVERRIDE;
	  virtual const char* classname() const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* custom_script_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
	  Bscript::BObject call_custom_method( const char* methodname, Bscript::BObjectImpRefVec& pmore );
	  Bscript::BObject call_custom_method( const char* methodname );

	protected: // only derived classes need the constructor
	  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual void printDebugProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;

	  Item( const ItemDesc& itemdesc, UOBJ_CLASS uobj_class );

	public:
	  Core::UContainer* container;
	protected:
	  unsigned int decayat_gameclock_;
	  u16 amount_;
	  bool newbie_;
	  bool movable_;
	  bool inuse_;
	  bool invisible_;

	  u8 slot_index_;

	  boost_utils::script_name_flystring on_use_script_;
      boost_utils::script_name_flystring equip_script_;
      boost_utils::script_name_flystring unequip_script_;
	  mutable const ItemDesc *_itemdesc;
	public:
	  u8 layer;
	  u8 tile_layer;
	  unsigned short hp_;
	  unsigned short maxhp() const;
      s16 maxhp_mod() const;
      void maxhp_mod(s16 newvalue);

	  double quality_;

	  s16 calc_element_resist( Core::ElementalType element ) const;
      s16 calc_element_damage( Core::ElementalType element ) const;
	  bool has_resistance( Mobile::Character* chr );
	  bool has_element_damage();

    private:
        Mobile::Character* gotten_by_;

    protected:
      static const u32 SELLPRICE_DEFAULT; // means use the itemdesc value
      static const u32 BUYPRICE_DEFAULT;
	};

	inline u16 Item::getamount() const
	{
	  return amount_;
	}

	inline bool Item::movable() const
	{
	  return movable_;
	}

	inline void Item::movable( bool newvalue )
	{
		if ( movable_ != newvalue )
		{
			movable_ = newvalue;
			on_movable_changed();
		}
	}

	inline bool Item::inuse() const
	{
	  return inuse_;
	}

	inline void Item::inuse( bool newvalue )
	{
		inuse_ = newvalue;
	}

	inline bool Item::is_gotten() const
	{
	  return gotten_by_ != nullptr;
	}

	inline void Item::set_gotten( Mobile::Character* by_char )
	{
		gotten_by_ = by_char;
	}

    inline Mobile::Character* Item::get_gotten() const
    {
      return gotten_by_;
    }

	inline bool Item::invisible() const
	{
	  return invisible_;
	}

	inline void Item::invisible( bool newvalue )
	{
		if ( invisible_ != newvalue )
		{
			invisible_ = newvalue;
			on_invisible_changed();
		}
	}

	inline bool Item::newbie() const
	{
	  return newbie_;
	}

	inline void Item::newbie( bool newvalue )
	{
		newbie_ = newvalue;
	}

	inline u8 Item::slot_index() const
	{
	  return slot_index_;
	}

    inline void Item::reset_slot()
    {
        slot_index_ = 0;
    }

	inline bool Item::slot_index( u8 newvalue )
	{
	  if ( Core::settingsManager.ssopt.use_slot_index )
	  {
		if ( newvalue < Core::settingsManager.ssopt.default_max_slots )
		{
		  slot_index_ = newvalue;
		  return true;
		}
		else
		  return false;
	  }
	  return true;
	}

    inline s16 Item::maxhp_mod() const {
        return getmember<s16>(Bscript::MBR_MAXHP_MOD);
    }
    inline void Item::maxhp_mod(s16 newvalue) {
        setmember<s16>(Bscript::MBR_MAXHP_MOD, newvalue);
    }

    inline std::string Item::name_suffix() const
    {
        return getmember<std::string>(Bscript::MBR_NAME_SUFFIX);
    }
    inline void Item::name_suffix(const std::string &suffix)
    {
        setmember<std::string>(Bscript::MBR_NAME_SUFFIX, suffix);
    }

    inline bool valid_equip_layer(int layer) {
        return layer >= Core::LAYER_INFO::LOWEST_LAYER && layer <= Core::LAYER_INFO::HIGHEST_LAYER;
    }

    inline bool valid_equip_layer(const Item* item) {
        return valid_equip_layer(item->tile_layer);
    }
  }
}
#endif
