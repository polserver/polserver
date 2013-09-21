/*
History
=======


Notes
=======

*/

#ifndef LOCKABLE_H
#define LOCKABLE_H

#include "item/item.h"

class ULockable : public Item
{
    typedef Item base;
public:
    bool locked() const;
protected:
    bool locked_;

	virtual void printProperties( fmt::Writer& writer ) const;
    virtual void printProperties( std::ostream& os ) const;
    virtual void readProperties( ConfigElem& elem );
    virtual class BObjectImp* get_script_member( const char* membername ) const;
    virtual class BObjectImp* get_script_member_id( const int id ) const; ///id test
    virtual class BObjectImp* set_script_member( const char* membername, int value );
    virtual class BObjectImp* set_script_member_id( const int id, int value ); //id test
    virtual bool script_isa( unsigned isatype ) const;

	virtual Item* clone() const; //dave 12-20
protected:
    explicit ULockable( const ItemDesc& itemdesc, UOBJ_CLASS uobj_class );
    friend class Item;
};

inline bool ULockable::locked() const
{
    return locked_;
}
#endif
