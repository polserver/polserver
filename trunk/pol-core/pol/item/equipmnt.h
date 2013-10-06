/*
History
=======


Notes
=======

*/

#ifndef EQUIPMNT_H
#define EQUIPMNT_H

#include <iosfwd>

#include "item.h"

class ConfigElem;
class EquipDesc;

class Equipment : public Item
{
    typedef Item base;

public:
    void reduce_hp_from_hit();

protected:
    Equipment( const ItemDesc& itemdesc, UOBJ_CLASS uobj_class );
    virtual void printProperties( StreamWriter& sw ) const;
    virtual void readProperties( ConfigElem& elem );
    virtual BObjectImp* get_script_member( const char *membername ) const;
    virtual BObjectImp* get_script_member_id( const int id ) const; ///id test
    //virtual BObjectImp* set_script_member( const char *membername, const std::string& value );
    virtual BObjectImp* set_script_member( const char *membername, int value );
    virtual BObjectImp* set_script_member_double( const char *membername, double value );
    virtual BObjectImp* set_script_member_id( const int id, int value ); //id test
    virtual BObjectImp* set_script_member_id_double( const int id, double value ); //id test

    virtual bool script_isa( unsigned isatype ) const;
    virtual Item* clone() const;

protected:
    const EquipDesc& eq_tmpl_;
};

#endif
