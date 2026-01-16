/** @file
 *
 * @par History
 * - 2005/07/01 Shinigami: added StoredConfigFile::modified_ (stat.st_mtime) to detect cfg file
 * modification
 * - 2005/07/04 Shinigami: added StoredConfigFile::reload to check for file modification
 *                         added ConfigFiles_log_stuff for memory logging
 */


#ifndef CFGREPOS_H
#define CFGREPOS_H

#include "pol_global_config.h"

#include <map>
#include <string>
#include <time.h>
#include <utility>

#include "../clib/boostutils.h"
#include "../clib/maputil.h"
#include "../clib/refptr.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
}
namespace Clib
{
class ConfigElem;
class ConfigFile;
}  // namespace Clib
namespace Module
{
class ConfigFileIterator;
}
namespace Core
{
class StoredConfigElem : public ref_counted
{
private:
  typedef std::multimap<boost_utils::cfg_key_flystring, ref_ptr<Bscript::BObjectImp>,
                        Clib::ci_cmp_pred>
      PropImpList;

public:
  StoredConfigElem() = default;
  StoredConfigElem( Clib::ConfigElem& elem );
  ~StoredConfigElem() override;

  StoredConfigElem( const StoredConfigElem& elem ) = delete;
  StoredConfigElem& operator=( const StoredConfigElem& ) = delete;

  size_t estimateSize() const;

  Bscript::BObjectImp* getimp( const std::string& propname ) const;
  Bscript::BObjectImp* listprops() const;
  void addprop( const std::string& propname, Bscript::BObjectImp* imp );

  typedef StoredConfigElem::PropImpList::const_iterator const_iterator;
  std::pair<const_iterator, const_iterator> equal_range( const std::string& propname ) const;

private:
  PropImpList propimps_;
};

class StoredConfigFile : public ref_counted
{
public:
  StoredConfigFile();
  //    ~StoredConfigFile();
  void load( Clib::ConfigFile& cf );
  void load_tus_scp( const std::string& filename );
  size_t estimateSize() const;

  typedef ref_ptr<StoredConfigElem> ElemRef;
  ElemRef findelem( int key );
  ElemRef findelem( const std::string& key );

  int maxintkey() const;
  time_t modified() const;

  typedef std::map<std::string, ElemRef, Clib::ci_cmp_pred> ElementsByName;
  ElementsByName::const_iterator byname_begin() { return elements_byname_.begin(); }
  ElementsByName::const_iterator byname_end() { return elements_byname_.end(); }
  typedef std::map<int, ElemRef> ElementsByNum;
  ElementsByNum::const_iterator bynum_begin() { return elements_bynum_.begin(); }
  ElementsByNum::const_iterator bynum_end() { return elements_bynum_.end(); }
  bool reload;  // try to reload cfg file?
  friend class Module::ConfigFileIterator;

private:
  ElementsByName elements_byname_;

  ElementsByNum elements_bynum_;

  time_t modified_;  // used to detect modification

  // not implemented:
  StoredConfigFile( const StoredConfigFile& ) = delete;
  StoredConfigFile& operator=( const StoredConfigFile& ) = delete;
};

typedef ref_ptr<StoredConfigFile> ConfigFileRef;

ConfigFileRef FindConfigFile( const std::string& filename, const std::string& allpkgbase );
void CreateEmptyStoredConfigFile( const std::string& filename );
int UnloadConfigFile( const std::string& filename );
ConfigFileRef LoadTusScpFile( const std::string& filename );

#ifdef MEMORYLEAK
void ConfigFiles_log_stuff();
#endif
}  // namespace Core
}  // namespace Pol

#endif
