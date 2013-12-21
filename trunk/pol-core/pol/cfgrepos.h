/*
History
=======
2005/07/01 Shinigami: added StoredConfigFile::modified_ (stat.st_mtime) to detect cfg file modification
2005/07/04 Shinigami: added StoredConfigFile::reload to check for file modification
added ConfigFiles_log_stuff for memory logging


Notes
=======

*/

#ifndef CFGREPOS_H
#define CFGREPOS_H

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include <map>
#include <string>

#include "../clib/maputil.h"
#include "../clib/refptr.h"
namespace Pol {
  namespace Bscript  {
	class BObjectImp;
  }
  namespace Clib {
	class ConfigFile;
	class ConfigElem;
  }
  namespace Core {

	class StoredConfigElem : public ref_counted
	{
	private:
	  typedef std::multimap<std::string, ref_ptr<Bscript::BObjectImp>, Clib::ci_cmp_pred > PropImpList;

	public:
	  StoredConfigElem( Clib::ConfigElem& elem );
	  StoredConfigElem();
	  ~StoredConfigElem();

	  Bscript::BObjectImp* getimp( const std::string& propname ) const;
	  Bscript::BObjectImp* listprops() const;
	  void addprop( const std::string& propname, Bscript::BObjectImp* imp );

	  typedef StoredConfigElem::PropImpList::const_iterator const_iterator;
	  pair<const_iterator, const_iterator> equal_range( const std::string& propname ) const;
	private:
	  PropImpList propimps_;

	  // not implemented:
	  StoredConfigElem( const StoredConfigElem& elem );
	  StoredConfigElem& operator=( const StoredConfigElem& );
	};

	class StoredConfigFile : public ref_counted
	{
	public:
	  StoredConfigFile();
	  //    ~StoredConfigFile();
	  void load( Clib::ConfigFile& cf );
	  void load_tus_scp( const std::string& filename );

	  typedef ref_ptr<StoredConfigElem> ElemRef;
	  ElemRef findelem( int key );
	  ElemRef findelem( const std::string& key );

	  int maxintkey() const;
	  const time_t modified() const;

	  typedef map<std::string, ElemRef, Clib::ci_cmp_pred> ElementsByName;
	  ElementsByName::const_iterator byname_begin() { return elements_byname_.begin(); }
	  ElementsByName::const_iterator byname_end() { return elements_byname_.end(); }

	  typedef map<int, ElemRef> ElementsByNum;
	  ElementsByNum::const_iterator bynum_begin() { return elements_bynum_.begin(); }
	  ElementsByNum::const_iterator bynum_end() { return elements_bynum_.end(); }

	  bool reload; // try to reload cfg file?
	private:
	  ElementsByName elements_byname_;

	  ElementsByNum elements_bynum_;

	  time_t modified_; // used to detect modification

	  // not implemented:
	  StoredConfigFile( const StoredConfigFile& );
	  StoredConfigFile& operator=( const StoredConfigFile& );
	};

	typedef ref_ptr<StoredConfigFile> ConfigFileRef;

	ConfigFileRef FindConfigFile( const std::string& filename, const std::string& allpkgbase );
	void CreateEmptyStoredConfigFile( const std::string& filename );
	int UnloadConfigFile( const std::string& filename );
	ConfigFileRef LoadTusScpFile( const std::string& filename );

#ifdef MEMORYLEAK
	void ConfigFiles_log_stuff();
#endif
  }
}

#endif
