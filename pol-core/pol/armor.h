/*
History
=======


Notes
=======

*/

#ifndef ARMOR_H
#define ARMOR_H

#include <iosfwd>

#include "equipmnt.h"
#include "item.h"
#include "scrdef.h"
#include "armrtmpl.h"


class ArmorDesc;

class UArmor : public Equipment
{
    typedef Equipment base;
public:
    virtual unsigned short ar() const;
    virtual bool covers( unsigned layer ) const;
    virtual Item* clone() const;
    void set_onhitscript( const string& scriptname );
	std::set<unsigned> tmplzones();
    
protected:
    virtual void printProperties( std::ostream& os ) const;
    virtual void readProperties( ConfigElem& elem );
    virtual BObjectImp* get_script_member( const char *membername ) const;
    virtual BObjectImp* get_script_member_id( const int id ) const; ///id test
    virtual BObjectImp* set_script_member( const char *membername, const std::string& value );
    virtual BObjectImp* set_script_member( const char *membername, long value );
    virtual BObjectImp* set_script_member_id( const int id, const std::string& value ); //id test
    virtual BObjectImp* set_script_member_id( const int id, long value );//id test
    virtual bool script_isa( unsigned isatype ) const;

    UArmor( const ArmorDesc& descriptor, const ArmorDesc* permanent_descriptor );
    friend class Item;
    //friend void load_data();

    const ArmorDesc& descriptor() const;
    const ScriptDef& onhitscript() const;


private:
    const ArmorDesc* tmpl;
    short ar_mod_;
    
    ScriptDef onhitscript_;
};

inline std::set<unsigned> UArmor::tmplzones() { return tmpl->zones; }

void load_armor_templates();
void unload_armor_templates();

#endif
