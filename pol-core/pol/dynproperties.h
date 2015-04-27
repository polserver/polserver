/*
History
=======

Notes
=======
*/

#ifndef __POL_DYNPROPS_H
#define __POL_DYNPROPS_H

#include "../clib/rawtypes.h"

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
  enum DynPropTypes : u32
  {
    EMPTY               = 0,
    AR_MOD              = 1 << 0,
    MAX_ITEMS_MOD       = 1 << 1,
    MAX_SLOTS_MOD       = 1 << 2,
    MAX_WEIGHT_MOD      = 1 << 3,
    SELLPRICE           = 1 << 4,
    BUYPRICE            = 1 << 5,
    MAXHP_MOD           = 1 << 6,
    NAME_SUFFIX         = 1 << 7,
    RESIST_FIRE         = 1 << 8,
    RESIST_COLD         = 1 << 9,
    RESIST_ENERGY       = 1 << 10,
    RESIST_POISON       = 1 << 11,
    RESIST_PHYSICAL     = 1 << 12,
    DAMAGE_FIRE         = 1 << 13,
    DAMAGE_COLD         = 1 << 14,
    DAMAGE_ENERGY       = 1 << 15,
    DAMAGE_POISON       = 1 << 16,
    DAMAGE_PHYSICAL     = 1 << 17,
    RESIST_FIRE_MOD     = 1 << 18,
    RESIST_COLD_MOD     = 1 << 19,
    RESIST_ENERGY_MOD   = 1 << 20,
    RESIST_POISON_MOD   = 1 << 21,
    RESIST_PHYSICAL_MOD = 1 << 22,
    DAMAGE_FIRE_MOD     = 1 << 23,
    DAMAGE_COLD_MOD     = 1 << 24,
    DAMAGE_ENERGY_MOD   = 1 << 25,
    DAMAGE_POISON_MOD   = 1 << 26,
    DAMAGE_PHYSICAL_MOD = 1 << 27,
    UNUSED1             = 1 << 28,
    UNUSED2             = 1 << 29,
    UNUSED3             = 1 << 30,
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
    PropHolder(DynPropTypes type, const boost::any& value);
    template <typename T>
    T getValue() const;
  protected:
    DynPropTypes _type;
    boost::any _value;
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
    void removeProperty(DynPropTypes type);
    size_t estimateSize() const;
  private:
    u32 _prop_bits;
    std::vector<PropHolder> _props;
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
    void setmember(DynPropTypes member, const T& value);
    template <typename T>
    void setmember(DynPropTypes member, const T& value, const T& defaultvalue);
    void removemember(DynPropTypes member);
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
  PropHolder::PropHolder() : 
    _type(DynPropTypes::EMPTY),
    _value()
  {}
  PropHolder::PropHolder(DynPropTypes type) :
    _type(type),
    _value()
  {}
  PropHolder::PropHolder(DynPropTypes type, const boost::any& value) :
    _type(type),
    _value(value)
  {}

  template <typename T>
  inline T PropHolder::getValue() const
  {
    return boost::any_cast<T>(_value);
  }

  ////////////////
  // DynProps
  DynProps::DynProps() :
    _prop_bits(DynPropTypes::EMPTY),
    _props()
  {}

  inline bool DynProps::hasProperty(DynPropTypes type) const
  {
	return (type & _prop_bits) != 0;
  }

  template <typename V>
  inline bool DynProps::getProperty(DynPropTypes type, V* value) const 
  {
    if (!hasProperty(type))
      return false;
    for (const PropHolder& prop : _props)
    {
	  if (prop._type == type)
      {
		*value = prop.getValue<V>();
        return true;
	  }
	}
    return false;
  }

  template <typename V>
  inline void DynProps::setProperty(DynPropTypes type, const V& value)
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
    _props.emplace_back(type,value);
  }

  inline void DynProps::removeProperty(DynPropTypes type)
  {
    if (!hasProperty(type))
      return;
    _props.erase(
      std::remove_if(_props.begin(), _props.end(),
        [&type](const PropHolder& x){return type == x._type;})
        , _props.end()),
    _prop_bits &= ~type;
  }

  inline size_t DynProps::estimateSize() const
  {
    return sizeof(u32)
      + 3 * sizeof(PropHolder*) + _props.capacity() * sizeof(PropHolder);
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
  void DynamicPropsHolder::setmember(DynPropTypes member, const T& value)
  {
    initProps();
    _dynprops->setProperty(member,value);
  }

  template <typename T>
  void DynamicPropsHolder::setmember(DynPropTypes member, const T& value, const T& defaultvalue)
  {
    if (value == defaultvalue)
    {
      if (_dynprops)
        _dynprops->removeProperty(member);
      return;
    }
    initProps();
    _dynprops->setProperty(member,value);
  }

  void DynamicPropsHolder::removemember(DynPropTypes member)
  {
    if (!hasmember(member))
      return;
    _dynprops->removeProperty(member);
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
