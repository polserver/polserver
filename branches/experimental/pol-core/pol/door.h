/*
History
=======


Notes
=======

*/

#ifndef __DOOR_H
#define __DOOR_H

#ifndef __ITEM_H
#	include "lockable.h"
#endif

class Client;
class DoorDesc;

class UDoor : public ULockable
{
    typedef ULockable base;
private:
    virtual void builtin_on_use( Client *client );
    void toggle();
    void open();
    void close();
    bool is_open() const;
    virtual BObjectImp* get_script_member( const char *membername ) const;
    virtual BObjectImp* get_script_member_id( const int id ) const; ///id test
    virtual BObjectImp* script_method( const char* methodname, Executor& ex );
    virtual BObjectImp* script_method_id( const int id, Executor& ex );
    virtual bool script_isa( unsigned isatype ) const;

protected:
    UDoor( const DoorDesc& descriptor );
    friend class Item;
};
#endif
