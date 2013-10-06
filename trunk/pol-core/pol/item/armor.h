/*
History
=======


Notes
=======

*/

#ifndef ARMOR_H
#define ARMOR_H

#include <iosfwd>

#include "../scrdef.h"

#include "equipmnt.h"
#include "item.h"
#include "armrtmpl.h"


class ArmorDesc;

class UArmor : public Equipment
{
    typedef Equipment base;
public:
    virtual unsigned short ar() const;
    virtual bool covers( unsigned short layer ) const;
    virtual Item* clone() const;
    void set_onhitscript( const std::string& scriptname );
	std::set<unsigned short> tmplzones();
    
protected:
	
    virtual void printProperties( StreamWriter& sw ) const;
    virtual void readProperties( ConfigElem& elem );
    virtual BObjectImp* get_script_member( const char *membername ) const;
    virtual BObjectImp* get_script_member_id( const int id ) const; ///id test
    virtual BObjectImp* set_script_member( const char *membername, const std::string& value );
    virtual BObjectImp* set_script_member( const char *membername, int value );
    virtual BObjectImp* set_script_member_id( const int id, const std::string& value ); //id test
    virtual BObjectImp* set_script_member_id( const int id, int value );//id test
    virtual bool script_isa( unsigned isatype ) const;

    UArmor( const ArmorDesc& descriptor, const ArmorDesc* permanent_descriptor );
    friend class Item;
    //friend void load_data();

    const ArmorDesc& descriptor() const;
    const ScriptDef& onhitscript() const;


private:
    const ArmorDesc* tmpl;
    
    ScriptDef onhitscript_;
};

inline std::set<unsigned short> UArmor::tmplzones() { return tmpl->zones; }

void load_armor_templates();
void unload_armor_templates();

#endif
