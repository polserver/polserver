/*
History
=======
2009/02/01 MuadDib:   Resistance storage added.

Notes
=======

*/


#ifndef __UOBJECT_H
#define __UOBJECT_H

#ifndef __UCONST_H
#	include "uconst.h"
#endif

#ifndef __CLIB_RAWTYPES_H
#	include "../clib/rawtypes.h"
#endif

#include "../clib/refptr.h"
#include "proplist.h"

#include "../clib/boostutils.h"

#include "../bscript/objmembers.h" // needed for knowing the definitions of MBR_...

#include <boost/any.hpp>
#include <boost/flyweight.hpp>

#include <iosfwd>
#include <map>
#include <string>
#include <atomic>
#include <set>

#include "../../lib/format/format.h"

#define pf_endl '\n'


namespace Pol {
  namespace Bscript {
	class BObjectImp;
	class Executor;
  }
  namespace Clib {
	class ConfigElem;
  }
  namespace Plib {
	class Realm;
  }
  namespace Items {
	class Item;
	class UArmor;
	class UWeapon;
  }
  namespace Mobile {
	class Character;
	class NPC;
  }
  namespace Multi {
	class UMulti;
	class UBoat;
  }
  namespace Core {
	class UContainer;
	class WornItemsContainer;

	// ULWObject: Lightweight object.
	// Should contain minimal data structures (and no virtuals)
	// Note, not yet needed, so nothing has been moved here.
	class ULWObject
	{};


#ifdef _MSC_VER
#	pragma pack( push, 1 )
#else
#	pragma pack(1)
#endif
	struct Resistances
	{
	  s16 fire;
	  s16 cold;
	  s16 poison;
	  s16 energy;
	  s16 physical;
	};

	struct Resistances_Mods
	{
	  s16 fire;
	  s16 cold;
	  s16 poison;
	  s16 energy;
	  s16 physical;
	};
	enum ElementalType
	{
	  ELEMENTAL_TYPE_MAX = 0x4,
	  ELEMENTAL_FIRE = 0x0,
	  ELEMENTAL_COLD = 0x1,
	  ELEMENTAL_ENERGY = 0x2,
	  ELEMENTAL_POISON = 0x3,
	  ELEMENTAL_PHYSICAL = 0x4
	};

	struct ElementDamages
	{
	  s16 fire;
	  s16 cold;
	  s16 poison;
	  s16 energy;
	  s16 physical;
	};

	struct ElementDamages_Mods
	{
	  s16 fire;
	  s16 cold;
	  s16 poison;
	  s16 energy;
	  s16 physical;
	};

	struct Expanded_Statbar
	{
	  static const s16 default_statcap = 225;
	  static const s16 default_luck = 0;
	  static const s8 default_followers = 0;
	  static const s8 default_followers_max = 0;
	  static const s16 default_dmg_min = 0;
	  static const s16 default_dmg_max = 0;
	  static const s32 default_tithing = 0;
	  Expanded_Statbar() :
		statcap( default_statcap ),
		luck( default_luck ),
		followers( default_followers ),
		followers_max( default_followers_max ),
		dmg_min( default_dmg_min ),
		dmg_max( default_dmg_max ),
		tithing( default_tithing )
	  {
	  };
	  s16 statcap;
	  s16 luck;
	  s8 followers;
	  s8 followers_max;
	  s16 dmg_min;
	  s16 dmg_max;
	  s32 tithing;
	};

	struct MovementCost_Mod
	{
	  MovementCost_Mod() :
		walk( 1.0 ),
		run( 1.0 ),
		walk_mounted( 1.0 ),
		run_mounted( 1.0 )
	  {
	  };
	  double walk;
	  double run;
	  double walk_mounted;
	  double run_mounted;
	};
#ifdef _MSC_VER
#	pragma pack( pop )
#else
#	pragma pack()
#endif

	template<typename T>
	struct MemberHelper
	{
	public:
	  static T getmember( const std::map<unsigned short, boost::any> &_map, unsigned short member )
	  {
		auto itr = _map.find( member );

		if ( itr == _map.end() )
		  return 0;
		else
		  return boost::any_cast<T>( ( *itr ).second );
	  };
	  static void setmember( std::map<unsigned short, boost::any> &_map, unsigned short member, T value )
	  {
		if ( value == 0 )
		  _map.erase( member );
		else
		  _map[member] = value;
	  };
	};
	template<>
	struct MemberHelper<std::string>
	{
	public:
	  static std::string getmember( const std::map<unsigned short, boost::any> &_map, unsigned short member )
	  {
		auto itr = _map.find( member );

		if ( itr == _map.end() )
		  return "";
		else
		  return boost::any_cast<std::string>( ( *itr ).second );
	  };
      static void setmember(std::map<unsigned short, boost::any> &_map, unsigned short member, std::string value)
	  {
		if ( value.empty() )
		  _map.erase( member );
		else
		  _map[member] = value;
	  };
	};

	/* NOTES:
			if you add fields, be sure to update Items::create().
			*/
	class UObject : protected ref_counted
	{
	public:
	  enum UOBJ_CLASS
	  {       // UOBJ_CLASS is meant to be coarse-grained.
		CLASS_ITEM,         // It's meant as an alternative to dynamic_cast.
		CLASS_CONTAINER,    //
		CLASS_CHARACTER,    // Mostly used to go from UItem to UContainer.
		CLASS_NPC,          //
		CLASS_WEAPON,
		CLASS_ARMOR,
		CLASS_MULTI
	  };


	  virtual std::string name() const;
	  virtual std::string description() const;

	  bool specific_name() const;
	  void setname( const std::string& );

	  bool getprop( const std::string& propname, std::string& propvalue ) const;
	  void setprop( const std::string& propname, const std::string& propvalue );
	  void eraseprop( const std::string& propname );
	  void copyprops( const UObject& obj );
	  void copyprops( const PropertyList& proplist );
	  void getpropnames( std::vector< std::string >& propnames ) const;
	  const PropertyList& getprops() const;
      
	  bool orphan() const;

	  virtual void destroy();

	  virtual u8 los_height() const = 0;
	  virtual unsigned int weight() const = 0;


	  virtual UObject* toplevel_owner(); // this isn't really right, it returns the WornItemsContainer
	  virtual UObject* owner();
	  virtual const UObject* owner() const;
	  virtual UObject* self_as_owner();
	  virtual const UObject* self_as_owner() const;
	  virtual const UObject* toplevel_owner() const;
	  virtual bool setgraphic( u16 newobjtype );
	  virtual bool setcolor( u16 newcolor );
	  virtual void on_color_changed();

	  virtual void setfacing( u8 newfacing ) = 0;
	  virtual void on_facing_changed();

	  virtual bool saveonexit() const;
	  virtual void saveonexit( bool newvalue );

	  virtual void printOn( Clib::StreamWriter& ) const;
	  virtual void printSelfOn( Clib::StreamWriter& sw ) const;

	  virtual void printOnDebug( Clib::StreamWriter& sw ) const;
	  virtual void readProperties( Clib::ConfigElem& elem );
	  //virtual Bscript::BObjectImp* script_member( const char *membername );
	  virtual Bscript::BObjectImp* make_ref() = 0;
	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const; ///id test

	  virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string& value );
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, int value );
	  virtual Bscript::BObjectImp* set_script_member_double( const char *membername, double value );

	  virtual Bscript::BObjectImp* set_script_member_id( const int id, const std::string& value );
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value );
	  virtual Bscript::BObjectImp* set_script_member_id_double( const int id, double value );

	  virtual Bscript::BObjectImp* script_method( const char *methodname, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex );

	  virtual Bscript::BObjectImp* custom_script_method( const char* methodname, Bscript::Executor& ex );
	  virtual bool script_isa( unsigned isatype ) const;
	  virtual const char* target_tag() const;

	  virtual const char *classname() const = 0;

      virtual size_t estimatedSize() const;


	  bool isa( UOBJ_CLASS uobj_class ) const;
	  bool ismobile() const;
	  bool isitem() const;
	  bool ismulti() const;

	  void ref_counted_add_ref();
	  void ref_counted_release();
	  unsigned ref_counted_count() const;
	  ref_counted* as_ref_counted() { return this; }

	  inline void increv() { _rev++; };
	  inline u32 rev() const { return _rev; };

	  bool dirty() const;
	  void set_dirty() { dirty_ = true; }
	  void clear_dirty() const;
	  static std::atomic<unsigned int> dirty_writes;
	  static std::atomic<unsigned int>  clean_writes;

      s16 getBaseResistance( ElementalType type ) const;
      void setBaseResistance( ElementalType type, s16 value );
      s16 getResistanceMod( ElementalType type ) const;
      void setResistanceMod( ElementalType type, s16 value );

      s16 getBaseElementDamage( ElementalType type ) const;
      void setBaseElementDamage( ElementalType type, s16 value );
      s16 getElementDamageMod( ElementalType type ) const;
      void setElementDamageMod( ElementalType type, s16 value );

	protected:

	  virtual void printProperties( Clib::StreamWriter& sw ) const;
	  virtual void printDebugProperties( Clib::StreamWriter& sw ) const;

	  UObject( u32 objtype, UOBJ_CLASS uobj_class );
	  virtual ~UObject();

	  friend class ref_ptr<UObject>;
	  friend class ref_ptr<Mobile::Character>;
	  friend class ref_ptr<Items::Item>;
	  friend class ref_ptr<Multi::UBoat>;
	  friend class ref_ptr<Multi::UMulti>;
	  friend class ref_ptr<Mobile::NPC>;
	  friend class ref_ptr<UContainer>;
	  friend class ref_ptr<Items::UWeapon>;
	  friend class ref_ptr<Items::UArmor>;
	  friend class ref_ptr<WornItemsContainer>;

	  friend class UObjectHelper;


	  // DATA:
	public:
	  u32 serial, serial_ext;

	  const u32 objtype_;
	  u16 graphic;
	  u16 color;
	  u16 x, y;
	  s8 z;
	  u8 height;

	  u8 facing; // not always used for items.
	  // always used for characters
	  Plib::Realm* realm;

	  bool saveonexit_;	// 1-25-2009 MuadDib added. So far only items will make use of this.
	  // Another possibility is adding this to NPCs for WoW style Instances.

	private:
	  const u8 uobj_class_;
	  mutable bool dirty_;
	  u32 _rev;

	protected:
      boost_utils::object_name_flystring name_;
      
      template <typename T>
      T getmember(unsigned short member) const
      {
          return MemberHelper<T>::getmember(dynmap, member);
      }
      template <typename T>
      void setmember(unsigned short member, T value)
      {
          MemberHelper<T>::setmember(dynmap, member, value);
      }

	private:
	  PropertyList proplist_;
	  std::map<unsigned short, boost::any> dynmap;
      
	private: // not implemented:
	  UObject( const UObject& );
	  UObject& operator=( const UObject& );
	};

	extern Clib::StreamWriter& operator << ( Clib::StreamWriter&, const UObject& );

	inline bool UObject::specific_name() const
	{
	  return !name_.get().empty();
	}

	inline bool UObject::isa( UOBJ_CLASS uobj_class ) const
	{
	  return uobj_class_ == uobj_class;
	}

	inline bool UObject::ismobile() const
	{
	  return ( uobj_class_ == CLASS_CHARACTER ||
			   uobj_class_ == CLASS_NPC );
	}

	inline bool UObject::isitem() const
	{
	  return !ismobile();
	}
	inline bool UObject::ismulti() const
	{
	  return ( uobj_class_ == CLASS_MULTI );
	}

	inline bool UObject::orphan() const
	{
	  return ( serial == 0 );
	}

	inline void UObject::ref_counted_add_ref()
	{
	  ref_counted::add_ref( REFERER_PARAM( this ) );
	}

	inline void UObject::ref_counted_release()
	{
	  ref_counted::release( REFERER_PARAM( this ) );
	}

	inline unsigned UObject::ref_counted_count() const
	{
	  return ref_counted::count();
	}

	inline bool IsCharacter( u32 serial )
	{
	  return ( ~serial & 0x40000000Lu ) ? true : false;
	}

	inline bool IsItem( u32 serial )
	{
	  return ( serial & 0x40000000Lu ) ? true : false;
	}
  }
}

#endif
