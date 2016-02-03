/** @file
 *
 * @par History
 * - 2005/05/25 Shinigami: added PropertyList::printProperties( ConfigElem& elem )
 * - 2016/01/31 Bodom:     added profiling support
 */


#ifndef PROPLIST_H
#define PROPLIST_H

#include "../clib/boostutils.h"
#include "../clib/rawtypes.h"
#include "../clib/spinlock.h"

#include <boost/flyweight.hpp>

#include <map>
#include <string>
#include <vector>
#include <set>


namespace Pol {
  namespace Bscript {
    class Executor;
    class BObjectImp;
  }
  namespace Clib {
    class StreamWriter;
    class ConfigElem;
  }
  namespace Core {
    class PropertyList;

    /**
     * Profiler for CProps: stores usage information and computes statistics
     *
     * @author Bodom
     */
    class CPropProfiler
    {
    public:

      /**
       * Type of CProp, for profiling
       */
      enum class Type : u8
      {
        //main types
        ACCOUNT,
        GUILD,
        GLOBAL,
        ITEM,
        MOBILE,
        MULTI,
        PARTY,

        //ignored types
        DATAFILEELEMENT,
        REGION,

        ///unknown type, do not use (internally used when profiler is enabled after startup)
        UNKNOWN,
      };

      /** Returns an instance of the profiler, instantiate it on first need */
      inline static CPropProfiler& instance() {
        static CPropProfiler instance;
        return instance;
      };

      /** No copies allowed */
      CPropProfiler(const CPropProfiler&) = delete;
      /** No copies allowed */
      void operator=(const CPropProfiler&) = delete;

      void registerProplist(const PropertyList* proplist, const Type type);
      void registerProplist(const PropertyList* proplist, const PropertyList* copiedFrom);
      void unregisterProplist(const PropertyList* proplist);

      void clear();
      void dumpProfile(std::ostream& os);

      size_t estimateSize();

    private:
      class HitsCounter
      {
      public:
        static const size_t READ = 0;
        static const size_t WRITE = 1;
        static const size_t ERASE = 2;

        inline HitsCounter() : hits(std::array<u64,3>{}) {};
        inline u64& operator[](size_t idx) { return hits[idx]; };
        inline const u64& operator[](size_t idx) const { return hits[idx]; };

        inline size_t sizeEstimate() const { return sizeof(void*) + sizeof(u64) * hits.size(); };
      private:
        /// 0=read, 1=write, 2=erase
        std::array<u64,3> hits;
      };
      typedef std::map<const PropertyList*, const Type> PropLists;
      typedef std::map<const std::string,HitsCounter> HitsEntries;
      typedef std::map<const Type,HitsEntries> Hits;

      CPropProfiler() : _proplists(new PropLists()), _hits(new Hits()) {};

      /**
       * Returns proplist type, internal usage
       */
      inline Type getProplistType(const PropertyList* proplist)
      {
        PropLists::iterator el;
        {
          Clib::SpinLockGuard lock(_proplistsLock);
          el = _proplists->find(proplist);
        }

        if( el == _proplists->end() ) {
          /// Unknown should happen only when the profiler has been disabled and
          /// then re-enabled (including when it was disabled at startup and enabled
          /// later). In any other case, it's a bug.
          return Type::UNKNOWN;
        }

        return el->second;
      };

      /**
       * Returns wether a given type should be ignored, intenal usage
       */
      inline bool isIgnored(Type type)
      {
        if( type == Type::DATAFILEELEMENT || type == Type::REGION )
          return true;
        return false;
      };

      void cpropAction(const PropertyList* proplist, const std::string& name, const size_t key );

      std::unique_ptr<PropLists> _proplists;
      std::unique_ptr<Hits> _hits;
      Clib::SpinLock _proplistsLock;
      Clib::SpinLock _hitsLock;

    public:
      /**
       * Register a cprop read
       *
       * @param proplist Pointer to the registered list where this cprop resides
       * @param name Name of the cprop
       */
      inline void cpropRead(const PropertyList* proplist, const std::string& name) { cpropAction(proplist, name, HitsCounter::READ); };
      /**
       * Register a cprop write
       *
       * @param proplist Pointer to the registered list where this cprop resides
       * @param name Name of the cprop
       */
      inline void cpropWrite(const PropertyList* proplist, const std::string& name) { cpropAction(proplist, name, HitsCounter::WRITE); };
      /**
       * Register a cprop erase
       *
       * @param proplist Pointer to the registered list where this cprop resides
       * @param name Name of the cprop
       * @throws std::runtime_error When proplist is not registered
       */
      inline void cpropErase(const PropertyList* proplist, const std::string& name) { cpropAction(proplist, name, HitsCounter::ERASE); };
    };


    /**
     * Holds an object's CProps
     */
	class PropertyList
	{
	public:
      PropertyList() = delete;
      PropertyList( const CPropProfiler::Type& type );
      PropertyList( const CPropProfiler::Type& type, bool force );
	  PropertyList( const PropertyList& );  //dave added 1/26/3
	  bool getprop( const std::string& propname, std::string& propvalue ) const;
	  void setprop( const std::string& propname, const std::string& propvalue );
	  void eraseprop( const std::string& propname );
	  void copyprops( const PropertyList& proplist );
	  void getpropnames( std::vector< std::string >& propnames ) const;
	  void clear();
      size_t estimatedSize() const;

	  void printProperties( Clib::StreamWriter& sw ) const;
	  void printProperties( Clib::ConfigElem& elem ) const;
	  void readProperties( Clib::ConfigElem& elem );

	  void printPropertiesAsStrings( Clib::StreamWriter& sw ) const;
	  void readRemainingPropertiesAsStrings( Clib::ConfigElem& elem );

	  bool operator==( const PropertyList& ) const;
	  PropertyList& operator-( const std::set<std::string>& );  //dave added 1/26/3
	  void operator-=( const std::set<std::string>& );  //dave added 1/26/3
	protected:
      typedef std::map<boost_utils::cprop_name_flystring, boost_utils::cprop_value_flystring> Properties;

	  Properties properties;

	private:
	  friend class UObjectHelper;

	  // not implemented
	  PropertyList& operator=( const PropertyList& );
	};

	Bscript::BObjectImp* CallPropertyListMethod( PropertyList& proplist,
										const char* methodname,
										Bscript::Executor& ex,
										bool& changed );
	Bscript::BObjectImp* CallPropertyListMethod_id( PropertyList& proplist,
										   const int id,
										   Bscript::Executor& ex,
										   bool& changed );
  }
}
#endif
