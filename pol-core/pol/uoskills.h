/*
History
=======


Notes
=======

*/

#ifndef UOSKILLS_H
#define UOSKILLS_H

#include <string>

class Attribute;
class Package;

class UOSkill
{
public:
    UOSkill( const Package* pkg, ConfigElem& elem );
    UOSkill();

    bool inited;
    unsigned skillid;
    std::string attributename;
    const Attribute* pAttr;
    
    const Package* pkg;
};

const UOSkill& GetUOSkill( unsigned skillid );
void clean_skills();

#endif
