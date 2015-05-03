/*
History
=======

Notes
=======
Design decisions (64bit):
Since boost::any has a size of 8 a padding of 8 will be introduced which means that even if the prop type is a u8 full 16 bytes will be used
boost::variant is as big as the biggest possible value plus a type information member
in our case the biggest type to store in a variant is u32 thus only a padding of 4 exists: min/max size is 12 instead of 16
an empty vector still uses 24bytes moving the vectors into a pointer saves if unused 16bytes.

Layout:
Uobject
 - ptr DynProps
-> 8 bytes per object

DynProps
 - bitset (currently 4 bytes (more then 32 types = 8 bytes)
 - vector<PropHolder> variant version
 - ptr vector<PropHolder> any version
-> N "small" properties:
     4+24+12*N+8 = 36+12*N
-> +M "big" properties:
     +24+(16+unk)*M = (36+12*N) + (24+(16+unk)*M)
     unk is the type size which is stored in boost::any

TODO:
Is it worse it to combine e.g resistances struct?
5 * s16
24+12*5 -> 84
Combining means storage as boost::any:
24+(16+5*2) -> 50
But only valid if all 5 props are really set, if only 2 props are set variant is smaller-> 24+12*2=48
Different idea combine per resistance type mod and real value:
for variant its still 24+12*5 -> 84
any would use 24+(16+5*2)*2 -> 76 (but again only if all props are set)
Combining would use the u32 size for a prop in a variant without loss (2*s16)
-> 8 props can be stored with less space 

*/

#ifndef __POL_DYNPROPS_H
#define __POL_DYNPROPS_H

#include "../clib/rawtypes.h"

#include <bitset>
#include <string>
#include <type_traits>

#include <boost/variant.hpp>
#include <boost/any.hpp>

namespace Pol {
  namespace Core {

  // define to generate 3 methods for get/set/has
  #define DYN_PROPERTY(name, type, id, defaultvalue) \
    type name() const \
    { \
      type val; \
      if (getmember<type>(id,&val)) \
        return val; \
      return defaultvalue; \
    }; \
    void name(const type val) \
    { \
      setmember(id, val, static_cast<type>(defaultvalue)); \
    }; \
    bool has##name() const \
    { \
      return hasmember(id); \
    }



  // enum for the propertys
  enum DynPropTypes : u8
  {
    AR_MOD              = 0,
    MAX_ITEMS_MOD       = 1,
    MAX_SLOTS_MOD       = 2,
    MAX_WEIGHT_MOD      = 3,
    SELLPRICE           = 4,
    BUYPRICE            = 5,
    MAXHP_MOD           = 6,
    NAME_SUFFIX         = 7,
    RESIST_FIRE         = 8,
    RESIST_COLD         = 9,
    RESIST_ENERGY       = 10,
    RESIST_POISON       = 11,
    RESIST_PHYSICAL     = 12,
    DAMAGE_FIRE         = 13,
    DAMAGE_COLD         = 14,
    DAMAGE_ENERGY       = 15,
    DAMAGE_POISON       = 16,
    DAMAGE_PHYSICAL     = 17,
    RESIST_FIRE_MOD     = 18,
    RESIST_COLD_MOD     = 19,
    RESIST_ENERGY_MOD   = 20,
    RESIST_POISON_MOD   = 21,
    RESIST_PHYSICAL_MOD = 22,
    DAMAGE_FIRE_MOD     = 23,
    DAMAGE_COLD_MOD     = 24,
    DAMAGE_ENERGY_MOD   = 25,
    DAMAGE_POISON_MOD   = 26,
    DAMAGE_PHYSICAL_MOD = 27,

    FLAG_SIZE // used for bitset size
  };

  template <typename Storage>
  class PropHolderContainer;

  // small property type no types above size 4, for bigger types boost::any will be used
  typedef boost::variant<u8,u16,u32,s8,s16,s32> variant_storage;

  // holder class
  // stores the property kind and via boost::variant/boost::any the value
  template <typename Storage>
  class PropHolder
  {
  public:
    template <typename S>
    friend class PropHolderContainer;
    explicit PropHolder(DynPropTypes type);
    PropHolder(DynPropTypes type, const Storage& value);
    template <typename V>
    V getValue() const;
  protected:
    DynPropTypes _type;
    Storage _value;
  };

  // Container class
  // simple vector of PropHolder instances, used to have common code between boost::any/variant
  template <typename Storage>
  class PropHolderContainer
  {
  public:
    PropHolderContainer();
    template <typename V>
    bool getValue(DynPropTypes type, V *value) const;
    template <typename V>
    bool updateValue(DynPropTypes type, const V &value);
    template <typename V>
    void addValue(DynPropTypes type, const V &value);
    void removeValue(DynPropTypes type);
    size_t estimateSize() const;
  private:
    std::vector<PropHolder<Storage>> _props;
  };

  // management class
  // has a bitset for fast checking if a property exists
  // simple vector for all the "small" PropHolders
  // and a lazy filled unique_ptr for "big" PropHolders
  class DynProps 
  {
    
  public:
    DynProps();
    // fast bitflag check
    bool hasProperty(DynPropTypes type) const;
    // get property returns false if non existent (checks via hasProperty before)
    template <typename V>
    bool getProperty(DynPropTypes type, V* value) const;
    // set property (sets also the flag)
    template <typename V>
    void setProperty(DynPropTypes type, const V& value);
    // remove a prop
    template <typename V>
    void removeProperty(DynPropTypes type);
    size_t estimateSize() const;
  private:
    std::bitset<FLAG_SIZE> _prop_bits;
    PropHolderContainer<variant_storage> _props;
    std::unique_ptr<PropHolderContainer<boost::any>> _any_props;
  };

  // Base class for dynamic properties, should be derived from
  // holds a pointer to the DynProps, which is lazy filled
  // due to memory usage (empty DynProps is larger then an unique_ptr)
  // TODO: should the ptr be release if the last member was removed?
  class DynamicPropsHolder {
  public:
    DynamicPropsHolder();
    bool hasmember(DynPropTypes member) const;
    template <typename V>
    bool getmember(DynPropTypes member, V* value) const;
    template <typename V>
    void setmember(DynPropTypes member, const V& value, const V& defaultvalue);
    size_t estimateSizeDynProps() const;

  private: 
    void initProps();
    std::unique_ptr<DynProps> _dynprops;
  };


  ////////////////////////////
  // Imp start
  ////////////////////////////

  ////////////////
  // PropHolder
  template <class Storage>
  PropHolder<Storage>::PropHolder(DynPropTypes type) :
    _type(type),
    _value()
  {}
  template <class Storage>
  PropHolder<Storage>::PropHolder(DynPropTypes type, const Storage& value) :
    _type(type),
    _value(value)
  {}

  template <>
  template <typename V>
  inline V PropHolder<boost::any>::getValue() const
  {
    return boost::any_cast<V>(_value);
  }
  template <>
  template <typename V>
  inline V PropHolder<variant_storage>::getValue() const
  {
    return boost::get<V>(_value);
  }

  ////////////////
  // PropHolderContainer
  template <class Storage>
  PropHolderContainer<Storage>::PropHolderContainer() :
    _props()
  {}

  template <class Storage>
  template <typename V>
  bool PropHolderContainer<Storage>::getValue(DynPropTypes type, V *value) const
  {
    for (const PropHolder<Storage>& prop : _props)
    {
      if ( prop._type == type )
      {
        *value = prop.template getValue<V>();
        return true;
      }
    }
    return false;
  }

  template <class Storage>
  template <typename V>
  bool PropHolderContainer<Storage>::updateValue(DynPropTypes type, const V &value)
  {
    for (PropHolder<Storage>& prop : _props)
    {
      if (prop._type == type)
      {
        prop._value = value;
        return true;
      }
    }
    return false;
  }
  template <class Storage>
  template <typename V>
  void PropHolderContainer<Storage>::addValue(DynPropTypes type, const V &value)
  {
    _props.emplace_back(type,value);
  }

  template <class Storage>
  void PropHolderContainer<Storage>::removeValue(DynPropTypes type)
  {
    _props.erase(
      std::remove_if(_props.begin(), _props.end(),
        [&type](const PropHolder<Storage>& x){ return type == x._type; })
        , _props.end());
  }
  
  template <class Storage>
  size_t PropHolderContainer<Storage>::estimateSize() const
  {
    return 3 * sizeof(void*) + _props.capacity() * sizeof(PropHolder<Storage>);
  }


  ////////////////
  // helper template functions to switch between both containers
  // TODO: single is_same are ugly as hell...
  namespace {
    template <typename V>
    static typename std::enable_if<
      std::is_same<u8, V>::value || std::is_same<s8, V>::value || std::is_same<u16, V>::value || std::is_same<s16, V>::value || std::is_same<u32, V>::value || std::is_same<s32, V>::value, bool>::type
    getPropertyHelper(const PropHolderContainer<variant_storage>& variant_props,
        const std::unique_ptr<PropHolderContainer<boost::any>>& any_props, DynPropTypes type, V* value)
    {
      (void)any_props;
      return variant_props.getValue(type, value);
    };
    template <typename V>
    static typename std::enable_if<
      !std::is_same<u8, V>::value && !std::is_same<s8, V>::value && !std::is_same<u16, V>::value && !std::is_same<s16, V>::value && !std::is_same<u32, V>::value && !std::is_same<s32, V>::value, bool>::type
    getPropertyHelper(const PropHolderContainer<variant_storage>& variant_props,
        const std::unique_ptr<PropHolderContainer<boost::any>>& any_props, DynPropTypes type, V* value)
    {
      (void)variant_props;
      passert_always(any_props);
      return any_props->getValue(type, value);
    };

    template <typename V>
    static typename std::enable_if<
      std::is_same<u8, V>::value || std::is_same<s8, V>::value || std::is_same<u16, V>::value || std::is_same<s16, V>::value || std::is_same<u32, V>::value || std::is_same<s32, V>::value, bool>::type
    updatePropertyHelper(PropHolderContainer<variant_storage>& variant_props,
        std::unique_ptr<PropHolderContainer<boost::any>>& any_props, DynPropTypes type, const V& value)
    {
      (void)any_props;
      return variant_props.updateValue(type,value);
    };
    template <typename V>
    static typename std::enable_if<
      !std::is_same<u8, V>::value && !std::is_same<s8, V>::value && !std::is_same<u16, V>::value && !std::is_same<s16, V>::value && !std::is_same<u32, V>::value && !std::is_same<s32, V>::value, bool>::type
    updatePropertyHelper(PropHolderContainer<variant_storage>& variant_props,
        std::unique_ptr<PropHolderContainer<boost::any>>& any_props, DynPropTypes type, const V& value)
    {
      (void)variant_props;
      passert_always(any_props);
      return any_props->updateValue(type,value);
    };
    template <typename V>
    static typename std::enable_if<
      std::is_same<u8, V>::value || std::is_same<s8, V>::value || std::is_same<u16, V>::value || std::is_same<s16, V>::value || std::is_same<u32, V>::value || std::is_same<s32, V>::value, void>::type
    addPropertyHelper(PropHolderContainer<variant_storage>& variant_props,
        std::unique_ptr<PropHolderContainer<boost::any>>& any_props, DynPropTypes type, const V& value)
    {
      (void)any_props;
      variant_props.addValue(type,value);
    };
    template <typename V>
    static typename std::enable_if<
      !std::is_same<u8, V>::value && !std::is_same<s8, V>::value && !std::is_same<u16, V>::value && !std::is_same<s16, V>::value && !std::is_same<u32, V>::value && !std::is_same<s32, V>::value, void>::type
    addPropertyHelper(PropHolderContainer<variant_storage>& variant_props,
        std::unique_ptr<PropHolderContainer<boost::any>>& any_props, DynPropTypes type, const V& value)
    {
      (void)variant_props;
      if (!any_props)
        any_props.reset(new PropHolderContainer<boost::any>());
      any_props->addValue(type,value);
    };

    template <typename V>
    static typename std::enable_if<
      std::is_same<u8, V>::value || std::is_same<s8, V>::value || std::is_same<u16, V>::value || std::is_same<s16, V>::value || std::is_same<u32, V>::value || std::is_same<s32, V>::value, void>::type
    removePropertyHelper(PropHolderContainer<variant_storage>& variant_props,
        std::unique_ptr<PropHolderContainer<boost::any>>& any_props, DynPropTypes type)
    {
      (void)any_props;
      variant_props.removeValue(type);
    };
    template <typename V>
    static typename std::enable_if<
      !std::is_same<u8, V>::value && !std::is_same<s8, V>::value && !std::is_same<u16, V>::value && !std::is_same<s16, V>::value && !std::is_same<u32, V>::value && !std::is_same<s32, V>::value, void>::type
    removePropertyHelper(PropHolderContainer<variant_storage>& variant_props,
        std::unique_ptr<PropHolderContainer<boost::any>>& any_props, DynPropTypes type)
    {
      (void)variant_props;
      passert_always(any_props);
      any_props->removeValue(type);
    };
  } // namespace

  ////////////////
  // DynProps
  DynProps::DynProps() :
    _prop_bits(),
    _props(),
    _any_props(nullptr)
  {}

  inline bool DynProps::hasProperty(DynPropTypes type) const
  {
    return _prop_bits.test(type);
  }
  template <typename V>
  inline bool DynProps::getProperty(DynPropTypes type, V* value) const 
  {
    if (!hasProperty(type))
      return false;
    return getPropertyHelper<V>(_props,_any_props, type, value);
  }

  template <typename V>
  inline void DynProps::setProperty(DynPropTypes type, const V& value)
  {
    if (hasProperty(type))
    {
      bool res = updatePropertyHelper<V>(_props,_any_props, type, value);
      passert_always(res);
      return;
    }
    _prop_bits.set(type,true);
    addPropertyHelper<V>(_props,_any_props, type, value);
  }

  template <typename V>
  inline void DynProps::removeProperty(DynPropTypes type)
  {
    if (!hasProperty(type))
      return;
    removePropertyHelper<V>(_props,_any_props,type);
    _prop_bits.set(type, false);
  }

  inline size_t DynProps::estimateSize() const
  {
    size_t size = sizeof(std::bitset<FLAG_SIZE>) + sizeof(std::unique_ptr<void>)
      + _props.estimateSize();
    if (_any_props)
      size += _any_props->estimateSize();
    return size;
  }

  ////////////////
  // DynamicPropsHolder

  DynamicPropsHolder::DynamicPropsHolder() :
    _dynprops(nullptr)
  {}

  void DynamicPropsHolder::initProps()
  {
    if (!_dynprops)
      _dynprops.reset(new DynProps());
  }

  template <typename V>
  bool DynamicPropsHolder::getmember(DynPropTypes member, V* value) const
  {
    if (!_dynprops)
      return false;
    return _dynprops->getProperty(member,value);
  }

  bool DynamicPropsHolder::hasmember(DynPropTypes member) const
  {
    if (!_dynprops || !_dynprops->hasProperty(member))
      return false;
    return true;
  }

  template <typename V>
  void DynamicPropsHolder::setmember(DynPropTypes member, const V& value, const V& defaultvalue)
  {
    if (value == defaultvalue)
    {
      if (_dynprops)
        _dynprops->removeProperty<V>(member);
      return;
    }
    initProps();
    _dynprops->setProperty(member,value);
  }

  inline size_t DynamicPropsHolder::estimateSizeDynProps() const
  {
    size_t size = sizeof(DynamicPropsHolder);
    if (_dynprops)
      size += _dynprops->estimateSize();
    return size;
  }


  } // namespace Core
} // namespace Pol

#endif
