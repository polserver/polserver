/*
History
=======


Notes
=======

*/

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "scrdef.h"

class ConfigElem;
class Package;
class ExportedFunction;

class Attribute
{
public:
	Attribute( const Package* pkg, ConfigElem& elem );
	explicit Attribute( const string& name );

	const Package* pkg;
	string name;
	unsigned attrid;

	vector< string > aliases; // aliases[0] is always name

	Attribute* next;

	ExportedFunction* getintrinsicmod_func;

	// UseSkill information
	unsigned delay_seconds;
	bool unhides;

	//Default cap
	unsigned short default_cap;

	ScriptDef script_;
};

Attribute* FindAttribute( const string& str );
Attribute* FindAttribute( unsigned attrid );
void clean_attributes();
extern vector< Attribute* > attributes;
extern unsigned numAttributes;

const unsigned ATTRIBUTE_MIN_EFFECTIVE = 0;
const unsigned ATTRIBUTE_MAX_EFFECTIVE = 6000;

const unsigned ATTRIBUTE_MIN_BASE = 0;
const unsigned ATTRIBUTE_MAX_BASE = 60000;

const short ATTRIBUTE_MIN_TEMP_MOD = -30000;
const short ATTRIBUTE_MAX_TEMP_MOD = +30000;

const short ATTRIBUTE_MIN_INTRINSIC_MOD = -30000;
const short ATTRIBUTE_MAX_INTRINSIC_MOD = +30000;

extern const Attribute* pAttrStrength;
extern const Attribute* pAttrIntelligence;
extern const Attribute* pAttrDexterity;

extern const Attribute* pAttrParry;
extern const Attribute* pAttrTactics;

#endif
