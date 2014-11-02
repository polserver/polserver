/*
History
=======
2010/02/03 Turley:    MethodScript support for mobiles


Notes
=======

*/

#ifndef NPCTMPL_H
#define NPCTMPL_H

#include <map>
#include <string>

namespace Pol {
  namespace Clib {
	class ConfigElem;
  }
  namespace Plib {
	class Package;
  }
  namespace Items {
	class UWeapon;
  }
  namespace Core {
	class ExportScript;

	class NpcTemplate
	{
	public:
	  std::string name;

	  Items::UWeapon* intrinsic_weapon;
	  const Plib::Package* pkg;

	  // std::string script;
	  enum ALIGNMENT { NEUTRAL, EVIL, GOOD } alignment;

	  ExportScript* method_script;

	  NpcTemplate( const Clib::ConfigElem& elem, const Plib::Package* pkg );

	  ~NpcTemplate();
	};

	typedef std::map< std::string, NpcTemplate* > NpcTemplates;

	extern NpcTemplates npc_templates;

	const NpcTemplate& find_npc_template( const Clib::ConfigElem& elem );
  }
}
#endif
