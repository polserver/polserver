/*
History
=======


Notes
=======

*/

#ifndef DATASTOREIMP_H
#define DATASTOREIMP_H


#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/fileutil.h"
#include "../../clib/streamsaver.h"

#include "../../bscript/bobject.h"

#include "../proplist.h"

#include <string>
#include <map>

namespace Pol {
  namespace Plib {
	class Package;
  }
  namespace Module {
	class DataFileElement : public ref_counted
	{
	public:
	  DataFileElement();
	  explicit DataFileElement( Clib::ConfigElem& elem );
	  void printOn( Clib::StreamWriter& sw ) const;

	  Core::PropertyList proplist;
	};
	typedef ref_ptr<DataFileElement> DataFileElementRef;

	//const int DF_KEYTYPE_STRING = 0x00; // currently unneeded
	const int DF_KEYTYPE_INTEGER = 0x01;

	class DataFileContents : public ref_counted
	{
	public:
	  DataFileContents( DataStoreFile* dsf );
	  virtual ~DataFileContents();
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
	  typedef std::map< std::string, DataFileElementRef, Clib::ci_cmp_pred > ElementsByString;
	  typedef std::map< int, DataFileElementRef > ElementsByInteger;

	  ElementsByString elements_by_string;
	  ElementsByInteger elements_by_integer;
	};
	typedef ref_ptr<DataFileContents> DataFileContentsRef;

	
	typedef Bscript::BApplicObj< DataFileContentsRef > DataFileRefObjImpBase;

	class DataFileRefObjImp : public DataFileRefObjImpBase
	{
	public:
	  explicit DataFileRefObjImp( DataFileContentsRef dfref );

	  virtual const char* typeOf() const POL_OVERRIDE;
	  virtual u8 typeOfInt() const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* copy() const POL_OVERRIDE;

	  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false ) POL_OVERRIDE;

	};

	
	class DataFileElemObj
	{
	public:
	  DataFileElemObj( DataFileContentsRef dfcontents, DataFileElementRef dfelem ) :
		dfcontents( dfcontents ), dfelem( dfelem )
	  {}
	public:
	  DataFileContentsRef dfcontents;
	  DataFileElementRef dfelem;
	};

	typedef Bscript::BApplicObj< DataFileElemObj > DataElemRefObjImpBase;
	class DataElemRefObjImp : public DataElemRefObjImpBase
	{
	public:
	  DataElemRefObjImp( DataFileContentsRef dfcontents, DataFileElementRef dflem );
	  virtual const char* typeOf() const POL_OVERRIDE;
	  virtual u8 typeOfInt() const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* copy() const POL_OVERRIDE;

	  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false ) POL_OVERRIDE;
	};

	class DataStoreFile
	{
	public:
	  explicit DataStoreFile( Clib::ConfigElem& elem );
	  DataStoreFile( const std::string& descriptor,
					 const Plib::Package* pkg,
					 const std::string& name,
					 int flags );
	  virtual ~DataStoreFile();
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

  }
}

#endif
