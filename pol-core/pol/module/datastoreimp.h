/** @file
 *
 * @par History
 */


#ifndef DATASTOREIMP_H
#define DATASTOREIMP_H

#include "../../bscript/bobject.h"

#include "../proplist.h"

#include <map>
#include <string>

namespace Pol
{
namespace Clib
{
class ConfigElem;
class ConfigFile;
class StreamWriter;
}  // namespace Clib
namespace Plib
{
class Package;
}
namespace Module
{
class DataFileElement final : public ref_counted
{
public:
  DataFileElement();
  explicit DataFileElement( Clib::ConfigElem& elem );
  void printOn( Clib::StreamWriter& sw ) const;

  Core::PropertyList proplist;
};
typedef ref_ptr<DataFileElement> DataFileElementRef;

// const int DF_KEYTYPE_STRING = 0x00; // currently unneeded
const int DF_KEYTYPE_INTEGER = 0x01;

class DataFileContents final : public ref_counted
{
public:
  DataFileContents( DataStoreFile* dsf );
  ~DataFileContents() override;
  size_t estimateSize() const;

  void load( Clib::ConfigFile& cf );
  void save( Clib::StreamWriter& sw );

  Bscript::BObjectImp* methodCreateElement( int key );
  Bscript::BObjectImp* methodCreateElement( const std::string& key );

  Bscript::BObjectImp* methodFindElement( int key );
  Bscript::BObjectImp* methodFindElement( const std::string& key );

  Bscript::BObjectImp* methodDeleteElement( int key );
  Bscript::BObjectImp* methodDeleteElement( const std::string& key );

  Bscript::BObjectImp* methodKeys() const;

  DataStoreFile* dsf;
  bool dirty;

private:
  typedef std::map<std::string, DataFileElementRef, Clib::ci_cmp_pred> ElementsByString;
  typedef std::map<int, DataFileElementRef> ElementsByInteger;

  ElementsByString elements_by_string;
  ElementsByInteger elements_by_integer;
};
typedef ref_ptr<DataFileContents> DataFileContentsRef;


typedef Bscript::BApplicObj<DataFileContentsRef> DataFileRefObjImpBase;

class DataFileRefObjImp final : public DataFileRefObjImpBase
{
public:
  explicit DataFileRefObjImp( DataFileContentsRef dfref );

  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;

  Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) override;
  Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                       bool forcebuiltin = false ) override;
};


class DataFileElemObj
{
public:
  DataFileElemObj( DataFileContentsRef dfcontents, DataFileElementRef dfelem )
      : dfcontents( dfcontents ), dfelem( dfelem )
  {
  }

public:
  DataFileContentsRef dfcontents;
  DataFileElementRef dfelem;
};

typedef Bscript::BApplicObj<DataFileElemObj> DataElemRefObjImpBase;
class DataElemRefObjImp final : public DataElemRefObjImpBase
{
public:
  DataElemRefObjImp( DataFileContentsRef dfcontents, DataFileElementRef dflem );
  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;

  Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) override;
  Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                       bool forcebuiltin = false ) override;
};

class DataStoreFile
{
public:
  explicit DataStoreFile( Clib::ConfigElem& elem );
  DataStoreFile( const std::string& descriptor, const Plib::Package* pkg, const std::string& name,
                 int flags );
  ~DataStoreFile();
  size_t estimateSize() const;
  bool loaded() const;
  void load();
  void save() const;
  std::string filename() const;
  std::string filename( unsigned ver ) const;
  void printOn( Clib::StreamWriter& sw ) const;

  std::string descriptor;
  std::string name;

  std::string pkgname;
  const Plib::Package* pkg;
  unsigned version;
  unsigned oldversion;
  int flags;
  bool unload;

  unsigned delversion;

  DataFileContentsRef dfcontents;
};
}  // namespace Module
}  // namespace Pol

#endif
