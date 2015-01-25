/*
History
=======
2009/12/02 Turley:    added config.max_tile_id - Tomi


Notes
=======

*/

#include "poltest.h"

#include "../clib/logfacility.h"
#include "../clib/passert.h"

#include "../plib/maptile.h"
#include "../plib/realm.h"
#include "../plib/testenv.h"

#include "realms.h"

#include "item/item.h"
#include "los.h"
#include "mobile/npc.h"
#include "uofile.h"
#include "polcfg.h"
#include "udatfile.h"
#include "globals/uvars.h"
#include "skilladv.h"

#include "uobject.h"
#include "../clib/timer.h"

#include <boost/variant.hpp>
#include <boost/any.hpp>

namespace Pol {
  namespace Plib {
    void pol_walk_test();
    void pol_drop_test();
    void pol_los_test();
    void pol_test_multiwalk();
    void display_test_results();
    void create_test_environment( );
    void inc_failures();
    void inc_successes( );
  }
  namespace Multi {
    void read_multidefs( );
  }
  namespace Core {
    //////////////////////////////////////////////////
    // Test Implementation of a new way to store dynamic props
    // Basic Idea:
    // instead of a big map, use a small vector plus a bitflag for fast checking
    // create the class only if needed for each object
    // roll a dice if any or variant should be used? seems to be equally fast, but variant can check at compiletime
#define TESTING_VARIANT
    namespace Testing {
    // enum for the propertys
    enum DynPropTypes : u32 {
	  PROP_TYPE_NONE = 0,
	  PROP_TYPE_NAME = 1,
	  PROP_TYPE_BLAH = 2,
    };

    class DynProps;
    // holder class
    // stores the property kind and via boost::any the value
    class PropHolder
    {
    public:
      friend class DynProps;
      PropHolder();
      explicit PropHolder(DynPropTypes type);
      template <typename T>
      inline T getValue() const;
    protected:
      DynPropTypes _type;
#ifdef TESTING_VARIANT
      boost::variant<u32,std::string> _value;
#else
      boost::any _value;
#endif
    };


    // management class
    // has a bitflag for fast checking if a property exists
    // simple vector for all the PropHolders
    class DynProps {
    public:
      DynProps();
      // fast bitflag check
      inline bool hasProperty(DynPropTypes type) const;
      // get property returns false if non existent (checks via hasProperty before)
      template <typename T>
      inline bool getProperty(DynPropTypes type, T* value) const;
      // set property (sets also the flag)
      template <typename T>
      inline void setProperty(DynPropTypes type, T value);
      // remove a prop
      inline void removeProperty(DynPropTypes type);
    private:
      u32 _prop_bits;
      std::vector<PropHolder> _props;
    };

    // example implementation (should later be UObject)
    // holds a pointer to the DynProps, which is lazy filled
    // due to memory usage, pointer is 8 but due to the vector the pure class is 32 (on my system)
    class NewPropUser {
    public:
      NewPropUser();
      template <typename T>
      T getmember(DynPropTypes member) const;
      template <typename T>
      void setmember(DynPropTypes member, T value);

    private:
      std::unique_ptr<DynProps> _dynprops;
    };

    PropHolder::PropHolder() 
      : _type(PROP_TYPE_NONE) ,
      _value()
      
    {}
    PropHolder::PropHolder(DynPropTypes type) 
      : _type(type),
      _value()
    {}

    template <typename T>
    inline T PropHolder::getValue() const
    {
#ifdef TESTING_VARIANT
      return boost::get<T>(_value);
#else
      return boost::any_cast<T>( _value );
#endif
    }

    DynProps::DynProps()
      : _prop_bits(0),
      _props()
    {
    }
    inline bool DynProps::hasProperty(DynPropTypes type) const
    {
	  return (type & _prop_bits) != 0;
	}
    template <typename T>
    inline bool DynProps::getProperty(DynPropTypes type, T* value) const 
    {
      if (!hasProperty(type))
        return false;
      for (const PropHolder& prop : _props)
      {
		if (prop._type == type)
        {
		  *value = prop.getValue<T>();
          return true;
		}
	  }
      return false;
    }
    template <typename T>
    inline void DynProps::setProperty(DynPropTypes type, T value)
    {
      if (hasProperty(type))
      {
        for (PropHolder& prop : _props) 
        {
		  if (prop._type == type) 
          {
			prop._value = value;
            return;
		  }
		}
      }
      _prop_bits |= type;
      _props.emplace_back(type);
      _props.back()._value = value;
    }

    inline void DynProps::removeProperty(DynPropTypes type)
    {
      if (!hasProperty(type))
        return;
      _props.erase(std::remove_if(_props.begin(), _props.end(),
        [&type](const PropHolder& x){return type == x._type;}),
        _props.end()),
      _prop_bits &= ~type;
    }

    NewPropUser::NewPropUser()
      : _dynprops(nullptr)
    {}
    template <typename T>
    T NewPropUser::getmember(DynPropTypes member) const
    {
      if (!_dynprops)
        return 0;
      T value;
      if (_dynprops->getProperty(member,&value))
        return value;
      return 0;
    }
    template <typename T>
    void NewPropUser::setmember(DynPropTypes member, T value)
    {
      if (!_dynprops)
        _dynprops.reset(new DynProps());
      _dynprops->setProperty(member,value);
    }

    // current UObject impl
    class OldMap {
    public:
      AnyMemberMap dynmap;
      template <typename T>
      T getmember(unsigned short member) const;
      template <typename T>
      bool getmember(unsigned short member, T* value) const;
      template <typename T>
      void setmember(unsigned short member, const T& value);
      template <typename T>
      void setmember(unsigned short member, const T& value, const T& defaultvalue);
    };
    template <typename T>
    inline T OldMap::getmember(unsigned short member) const
    {
        return MemberHelper<T>::getmember(dynmap, member);
    }
    template <typename T>
    inline bool OldMap::getmember(unsigned short member, T* value) const
    {
        return MemberHelper<T>::getmember(dynmap, member, value);
    }
    template <typename T>
    inline void OldMap::setmember(unsigned short member, const T& value)
    {
        MemberHelper<T>::setmember(dynmap, member, value);
    }
    template <typename T>
    inline void OldMap::setmember(unsigned short member, const T& value, const T& defaultvalue)
    {
      MemberHelper<T>::setmember(dynmap, member, value, defaultvalue);
    }

    u32 last_value;
    void test_prop_imps()
    {
      {
        // test access with old method
        std::vector<OldMap*> oldones;
        std::vector<u32> newones;
        for (u32 i=0;i<1000000;++i)
        {
          OldMap* m=new OldMap();
          oldones.push_back(m);
        }
        for (u32 i=0;i<1000000;++i)
        {
          OldMap* m=new OldMap();
          m->setmember<u32>(1,10);
          m->setmember<u32>(2,10);
          oldones.push_back(m);
        }
        {
          Tools::Timer<Tools::DebugT> t;
          for (const auto& m: oldones)
          {  
            last_value=m->getmember<u32>(2);
            if (last_value==0) //try to convince the compiler to not optimize out the whole loop
              newones.push_back(last_value);
          }
        }
        INFO_PRINT << newones.size()<<"\n";
      }

      {
        std::vector<NewPropUser*> oldones;
        std::vector<u32> newones;
        for (u32 i=0;i<1000000;++i)
        {
          NewPropUser* m=new NewPropUser();
          oldones.push_back(m);
        }
        for (u32 i=0;i<1000000;++i)
        {
          NewPropUser* m=new NewPropUser();
          m->setmember<u32>(PROP_TYPE_NAME,10);
          m->setmember<u32>(PROP_TYPE_BLAH,10);
          oldones.push_back(m);
        }
        {
          Tools::Timer<Tools::DebugT> t;
          for (const auto& m: oldones)
          {  
            last_value=m->getmember<u32>(PROP_TYPE_BLAH);
            if (last_value==0)
              newones.push_back(last_value);
          }
        
        }
        INFO_PRINT << newones.size()<<"\n";
      }
    }
    } // namespace Testing

    extern bool static_debug_on;

	extern Mobile::NPC* test_banker;
	extern Items::Item* test_guard_door;
	extern Mobile::NPC* test_water_elemental;
	extern Mobile::NPC* test_seaserpent;

	void map_test()
	{
	  Plib::MAPTILE_CELL cell = gamestate.main_realm->getmaptile( 1453, 1794 );
      INFO_PRINT << cell.landtile << " " << cell.z << "\n";
	}

	void test_skilladv( unsigned int raw, unsigned short base )
	{
      INFO_PRINT << "Raw " << raw << ", base " << base << ": ";
	  if ( raw_to_base( raw ) != base )
	  {
        INFO_PRINT << "raw_to_base failed!\n";
        Plib::inc_failures( );
	  }
	  if ( base_to_raw( base ) != raw )
	  {
        INFO_PRINT << "base_to_raw failed!\n";
        Plib::inc_failures( );
	  }
      INFO_PRINT << "Passed\n";
      Plib::inc_successes( );
	}

	void skilladv_test()
	{
      INFO_PRINT << "Skill advancement tests:\n";
	  test_skilladv( 20, 1 );
	  test_skilladv( 512, 25 );
	  test_skilladv( 1024, 50 );
	  test_skilladv( 2048, 100 );
	  test_skilladv( 3072, 150 );
	  test_skilladv( 4096, 200 );
	  test_skilladv( 5120, 225 );
	  test_skilladv( 6144, 250 );
	  test_skilladv( 7168, 275 );
	  test_skilladv( 8192, 300 );
	  test_skilladv( 16384, 400 );
	  test_skilladv( 32768, 500 );
	  test_skilladv( 65536, 600 );
	  test_skilladv( 131072, 700 );
	  test_skilladv( 262144, 800 );
	  test_skilladv( 524288, 900 );
	  test_skilladv( 1048576, 1000 );
	  test_skilladv( 2097152, 1100 );
	  test_skilladv( 4194304, 1200 );
	  test_skilladv( 8388608, 1300 );
	  test_skilladv( 16777216, 1400 );
	  test_skilladv( 33554432, 1500 );
	  test_skilladv( 67108864, 1600 );
	  test_skilladv( 134217728, 1700 );
	  test_skilladv( 268435456, 1800 );
	  test_skilladv( 536870912, 1900 );

	  for ( unsigned short base = 0; base < 2000; ++base )
	  {
        INFO_PRINT << "Base " << base << ": Raw=";
		unsigned int raw = base_to_raw( base );
        INFO_PRINT << raw << ": ";
		unsigned short calcbase = raw_to_base( raw );
		if ( calcbase != base )
		{
          INFO_PRINT << "Failed (" << calcbase << ")\n";
          Plib::inc_failures( );
		}
		else
		{
          INFO_PRINT << "Passed\n";
          Plib::inc_successes( );
		}
	  }

	  for ( unsigned int raw = 0; raw < 2500; ++raw )
	  {
        INFO_PRINT << "Raw " << raw << ": Base=";
		unsigned short base = raw_to_base( raw );
        INFO_PRINT << base << ": ";
		unsigned int calcraw = base_to_raw( base );
		if ( calcraw != raw )
		{
          INFO_PRINT << "Failed (" << calcraw << ")\n";
          Plib::inc_failures( );
		}
		else
		{
          INFO_PRINT << "Passed\n";
          Plib::inc_successes( );
		}
	  }
	}

	void run_pol_tests()
	{
      Testing::test_prop_imps();
      return;
      Plib::create_test_environment();

	  Plib::pol_drop_test();
      Plib::pol_walk_test( );
      Plib::pol_los_test( );
      Plib::pol_test_multiwalk( );

      Plib::display_test_results( );
	}


	/* TO ADD:
	  (,5909,1395,30)  (walking through the floor?)
	  (2193,1408,-2) -- no LOS to (2191, 1412, -2) ?
	  */


#ifdef POLTEST
	Item* Item::create( u32 objtype, u32 serial )
	{
	  passert( objtype <= config.max_tile_id );
	  return new Item(objtype, CLASS_ITEM);
	}

	UMulti* UMulti::create(  u32 objtype, u32 serial )
	{
	  UMulti* multi = NULL;
	  if ( objtype <= config.max_tile_id)
		return NULL;

	  if (defs[objtype] != NULL)
	  {
		multi = new UMulti( objtype );
	  }
	  else
	  {
        ERROR_PRINT << "Tried to create multi " << objtype << " but no definition exists in MULTI.MUL\n";
		return NULL;
	  }

	  /*
		  if (serial)
		  multi->serial = UseItemSerialNumber( serial );
		  else
		  multi->serial = GetNewItemSerialNumber();

		  multi->serial_ext = ctBEu32( multi->serial );
		  */

	  return multi;
	}


	Items items;
	Multis multis;

	int main( int arc, char *argv[] )
	{
	  Plib::systemstate.config.uo_datafile_root = "C:/Games/AoS2D/";
      INFO_PRINT << "Opening UO data files..\n";
	  open_uo_data_files();
	  read_uo_data();
	  Multi::read_multidefs();

	  run_pol_tests();
	  clear_tiledata();
	  return 0;
	}
#endif
  }
}