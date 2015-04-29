/*
History
=======

Notes
=======
*/

#ifndef __POL_DYNPROPS_H
#define __POL_DYNPROPS_H

#include "../clib/rawtypes.h"

#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <string>

namespace Pol {
  namespace Core {
  
  // TODO:
  // should we split the types?
  // Pro: smaller vectors, especially the whole resist/damage stuff floods the props,
  //      if u32 limit for the flags is reached u64 needs to be used, increasing the size per prop even more
  // Contra: each instance needs 8 even if unused
  //
  // TODO:
  // How to get rid of the _type member per PropHolder instance?
  // my only idea currently is to add virtual inheritance plus pointer storage:
  // BaseClass with virtual getType(), PropHolder is Template -> derives from an empty class with just a static member (DynPropTypes)
  // additional downside is that setProperty then needs to have the empty class as argument instead of the enum value (to create a new attribute)
  // pointer storage is needed for the vector of props to allow the virtual call

  // TODO:
  // All the resistance/damage things: instead of storing each separated member, store the whole structure?
  // currently each attribute has an overhead of u32
  // -> having a char with all resistances set needs 5*(s16+u32) = 30
  //    having only the full struct: (5*s16)+u32 = 14
  //    the other way around storing single values would use less if only two or less values are needed


  // enum for the propertys
  enum DynPropTypes : u8
  {
    EMPTY               = 0,
    AR_MOD              = 1,
    MAX_ITEMS_MOD       = 2,
    MAX_SLOTS_MOD       = 3,
    MAX_WEIGHT_MOD      = 4,
    SELLPRICE           = 5,
    BUYPRICE            = 6,
    MAXHP_MOD           = 7,
    NAME_SUFFIX         = 8,
    RESIST_FIRE         = 9,
    RESIST_COLD         = 10,
    RESIST_ENERGY       = 11,
    RESIST_POISON       = 12,
    RESIST_PHYSICAL     = 13,
    DAMAGE_FIRE         = 14,
    DAMAGE_COLD         = 15,
    DAMAGE_ENERGY       = 16,
    DAMAGE_POISON       = 17,
    DAMAGE_PHYSICAL     = 18,
    RESIST_FIRE_MOD     = 19,
    RESIST_COLD_MOD     = 20,
    RESIST_ENERGY_MOD   = 21,
    RESIST_POISON_MOD   = 22,
    RESIST_PHYSICAL_MOD = 23,
    DAMAGE_FIRE_MOD     = 24,
    DAMAGE_COLD_MOD     = 25,
    DAMAGE_ENERGY_MOD   = 26,
    DAMAGE_POISON_MOD   = 27,
    DAMAGE_PHYSICAL_MOD = 28,
  };

  template <typename Storage>
  class PropHolderContainer;
  // holder class
  // stores the property kind and via boost::any the value
  typedef boost::variant<u8,u16,u32,s8,s16,s32> variant_storage;
  template <typename Storage>
  class PropHolder
  {
  public:
    template <typename S>
    friend class PropHolderContainer;
    PropHolder();
    explicit PropHolder(DynPropTypes type);
    PropHolder(DynPropTypes type, const Storage& value);
    template <typename T>
    T getValue() const;
  protected:
    DynPropTypes _type;
    Storage _value;
  };

  template <typename Storage>
  class PropHolderContainer
  {
  public:
    PropHolderContainer();
    template <typename T>
    bool getValue(DynPropTypes type, T *value) const;
    template <typename T>
    bool updateValue(DynPropTypes type, const T &value);
    template <typename T>
    void addValue(DynPropTypes type, const T &value);
    void removeValue(DynPropTypes type);
    size_t estimateSize() const;
  private:
    std::vector<PropHolder<Storage>> _props;
  };

  // management class
  // has a bitflag for fast checking if a property exists
  // simple vector for all the PropHolders
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
    u32 _prop_bits;
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
    template <typename T>
    bool getmember(DynPropTypes member, T* value) const;
    template <typename T>
    void setmember(DynPropTypes member, const T& value, const T& defaultvalue);
    size_t estimateSize() const;
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
  PropHolder<Storage>::PropHolder() : 
    _type(DynPropTypes::EMPTY),
    _value()
  {}
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
  template <typename T>
  inline T PropHolder<boost::any>::getValue() const
  {
    return boost::any_cast<T>(_value);
  }
  template <>
  template <typename T>
  inline T PropHolder<variant_storage>::getValue() const
  {
    return boost::get<T>(_value);
  }

  ////////////////
  // PropHolderContainer
  template <class Storage>
  PropHolderContainer<Storage>::PropHolderContainer() :
    _props()
  {}

  template <class Storage>
  template <typename T>
  bool PropHolderContainer<Storage>::getValue(DynPropTypes type, T *value) const
  {
    for (const PropHolder<Storage>& prop : _props)
    {
      if ( prop._type == type )
      {
        *value = prop.getValue<T>();
        return true;
      }
    }
    return false;
  }

  template <class Storage>
  template <typename T>
  bool PropHolderContainer<Storage>::updateValue(DynPropTypes type, const T &value)
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
  template <typename T>
  void PropHolderContainer<Storage>::addValue(DynPropTypes type, const T &value)
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
  // DynProps
  DynProps::DynProps() :
    _prop_bits(DynPropTypes::EMPTY),
    _props(),
    _any_props(nullptr)
  {}

  inline bool DynProps::hasProperty(DynPropTypes type) const
  {
    passert_always(type == DynPropTypes::EMPTY);
	return ((1 << (type-1)) & _prop_bits) != 0;
  }

  template <typename V>
  inline bool DynProps::getProperty(DynPropTypes type, V* value) const 
  {
    if (!hasProperty(type))
      return false;
    if (sizeof(V) <= sizeof(u32))
      return _props.getValue(type, value);
    else
    {
      passert_always(!_any_props);
      return _any_props->getValue(type, value);
    }
  }

  template <typename V>
  inline void DynProps::setProperty(DynPropTypes type, const V& value)
  {
    if (hasProperty(type))
    {
      if (sizeof(V) <= sizeof(u32))
      {
        passert_always(!_props.updateValue(type,value));
      }
      else
      {
        passert_always(!_any_props);
        passert_always(!_any_props->updateValue(type,value));
      }
      return;
    }
    _prop_bits |= (1 << (type-1));
    if (sizeof(V) <= sizeof(u32))
      _props.addValue(type,value);
    else
    {
      if (!_any_props)
        _any_props.reset(new PropHolderContainer<boost::any>());
      _any_props->addValue(type,value);
    }
  }

  template <typename V>
  inline void DynProps::removeProperty(DynPropTypes type)
  {
    if (!hasProperty(type))
      return;
    if (sizeof(V) <= sizeof(u32))
      _props.removeValue(type);
    else
    {
      passert_always(!_any_props);
      _any_props->removeValue(type);
    }
    _prop_bits &= ~(1 << (type-1));
  }

  inline size_t DynProps::estimateSize() const
  {
    size_t size = sizeof(u32) + sizeof(std::unique_ptr<void>)
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

  template <typename T>
  bool DynamicPropsHolder::getmember(DynPropTypes member, T* value) const
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

  template <typename T>
  void DynamicPropsHolder::setmember(DynPropTypes member, const T& value, const T& defaultvalue)
  {
    if (value == defaultvalue)
    {
      if (_dynprops)
        _dynprops->removeProperty<T>(member);
      return;
    }
    initProps();
    _dynprops->setProperty(member,value);
  }

  inline size_t DynamicPropsHolder::estimateSize() const
  {
    size_t size = sizeof(DynamicPropsHolder);
    if (_dynprops)
      size += _dynprops->estimateSize();
    return size;
  }


  } // namespace Core
} // namespace Pol

#endif
