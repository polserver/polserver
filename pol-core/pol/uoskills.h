/** @file
 *
 * @par History
 */


#ifndef UOSKILLS_H
#define UOSKILLS_H

#include <string>
namespace Pol {
  namespace Plib {
	class Package;
  }
  namespace Clib {
      class ConfigElem;
  }
  namespace Mobile {
	class Attribute;
  }
  namespace Core {

	class UOSkill
	{
	public:
	  UOSkill( const Plib::Package* pkg, Clib::ConfigElem& elem );
	  UOSkill();
      size_t estimateSize() const;

	  bool inited;
	  unsigned skillid;
	  std::string attributename;
	  const Mobile::Attribute* pAttr;

	  const Plib::Package* pkg;
	};

	const UOSkill& GetUOSkill( unsigned skillid );
	void clean_skills();
  }
}
#endif
