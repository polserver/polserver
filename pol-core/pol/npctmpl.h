/*
History
=======
2010/02/03 Turley:    MethodScript support for mobiles


Notes
=======

*/

#ifndef NPCTMPL_H
#define NPCTMPL_H

#include "../clib/cfgfile.h"
#include "../clib/cfgelem.h"

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
	class UArmor;
  }
  namespace Core {
	class ExportScript;

	class NpcTemplate
	{
	public:
	  std::string name;

	  Items::UWeapon* intrinsic_weapon;
	  Items::UArmor* intrinsic_shield;
	  const Plib::Package* pkg;

	  // std::string script;
	  enum ALIGNMENT : u8 { NEUTRAL, EVIL, GOOD } alignment;

	  ExportScript* method_script;

	  NpcTemplate( const Clib::ConfigElem& elem, const Plib::Package* pkg );
      size_t estimateSize() const;
	  ~NpcTemplate();
	};

	const NpcTemplate& find_npc_template( const Clib::ConfigElem& elem );

	class NpcTemplateConfigSource : public Clib::ConfigSource
	{
	public:
	  NpcTemplateConfigSource();
      NpcTemplateConfigSource( const Clib::ConfigFile& cf );
      size_t estimateSize() const;
	  virtual void display_error( const std::string& msg,
								  bool show_curline = true,
                                  const Clib::ConfigElemBase* elem = NULL,
								  bool error = true ) const POL_OVERRIDE;
	private:
	  std::string _filename;
	  unsigned _fileline;
	};

	class NpcTemplateElem
	{
	public:
	  NpcTemplateElem();
      NpcTemplateElem( const Clib::ConfigFile& cf, const Clib::ConfigElem& elem );
      size_t estimateSize() const;
      void copyto( Clib::ConfigElem& elem ) const;
	private:
	  NpcTemplateConfigSource _source;
      Clib::ConfigElem _elem;
	};
  }
}
#endif
