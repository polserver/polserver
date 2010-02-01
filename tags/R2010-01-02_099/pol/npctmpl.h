/*
History
=======


Notes
=======

*/

#ifndef NPCTMPL_H
#define NPCTMPL_H

#include <map>
#include <string>

class ConfigElem;
class Package;
class UWeapon;

class NpcTemplate
{
public:
    std::string name;
    
    UWeapon* intrinsic_weapon;
    const Package* pkg;

    // std::string script;
    enum ALIGNMENT { NEUTRAL, EVIL, GOOD } alignment;

    NpcTemplate( const ConfigElem& elem, const Package* pkg );
};

typedef std::map< std::string, NpcTemplate* > NpcTemplates;

extern NpcTemplates npc_templates;

const NpcTemplate& find_npc_template( const ConfigElem& elem );

#endif
