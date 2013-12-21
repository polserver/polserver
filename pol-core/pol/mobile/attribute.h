/*
History
=======


Notes
=======

*/

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "../scrdef.h"
namespace Pol {
  namespace Clib {
	class ConfigElem;
  }
  namespace Plib {
	class Package;
  }
  namespace Core {
	class ExportedFunction;
  }
  namespace Mobile {

	class Attribute
	{
	public:
	  Attribute( const Plib::Package* pkg, Clib::ConfigElem& elem );
	  explicit Attribute( const std::string& name );

	  const Plib::Package* pkg;
	  std::string name;
	  unsigned attrid;

	  vector< std::string > aliases; // aliases[0] is always name

	  Attribute* next;

	  Core::ExportedFunction* getintrinsicmod_func;

	  // UseSkill information
	  unsigned delay_seconds;
	  bool unhides;
	  bool disable_core_checks;

	  //Default cap
	  unsigned short default_cap;

	  Core::ScriptDef script_;
	};

	Attribute* FindAttribute( const std::string& str );
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
  }
}
#endif
