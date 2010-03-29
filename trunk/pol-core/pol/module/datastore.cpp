/*
History
=======
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2006/09/26 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
2007/06/17 Shinigami: added config.world_data_path
2009/12/21 Turley:    ._method() call fix

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include <fstream>


#include "../../bscript/berror.h"
#include "../../bscript/impstr.h"
#include "../../bscript/objmethods.h"

#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/fileutil.h"
#include "../../clib/maputil.h"
#include "../../clib/stlutil.h"

#include "datastore.h"
#include "../dtrace.h"
#include "../../plib/pkg.h"
#include "../proplist.h"
#include "../uoexhelp.h"

class DataFileElement;
class DataStoreFile;

///
/// Datastore
///
///  datastore files are stored in
///	   config.world_data_path + ds/fname.txt
///	   config.world_data_path + ds/{pkgname}/fname.txt
///

class DataFileElement : public ref_counted
{
public:
	DataFileElement();
	explicit DataFileElement( ConfigElem& elem );
	void printOn( ostream& os ) const;

	PropertyList proplist;
};
typedef ref_ptr<DataFileElement> DataFileElementRef;

const int DF_KEYTYPE_STRING = 0x00;
const int DF_KEYTYPE_INTEGER = 0x01;

class DataFileContents : public ref_counted
{
public:
	DataFileContents( DataStoreFile* dsf );
	virtual ~DataFileContents();

	void load( ConfigFile& cf );
	void save( ostream& os );

	BObjectImp* methodCreateElement( int key );
	BObjectImp* methodCreateElement( const string& key );

	BObjectImp* methodFindElement( int key );
	BObjectImp* methodFindElement( const string& key );

	BObjectImp* methodDeleteElement( int key );
	BObjectImp* methodDeleteElement( const string& key );

	BObjectImp* methodKeys() const;

	DataStoreFile* dsf;
	bool dirty;

private:
	typedef std::map< std::string, DataFileElementRef, ci_cmp_pred > ElementsByString;
	typedef std::map< int, DataFileElementRef > ElementsByInteger;

	ElementsByString elements_by_string;
	ElementsByInteger elements_by_integer;
};
typedef ref_ptr<DataFileContents> DataFileContentsRef;

BApplicObjType datafileref_type;
typedef BApplicObj< DataFileContentsRef > DataFileRefObjImpBase;

class DataFileRefObjImp : public DataFileRefObjImpBase
{
public:
	explicit DataFileRefObjImp( DataFileContentsRef dfref );

	virtual const char* typeOf() const;
	virtual BObjectImp* copy() const;

	virtual BObjectImp* call_method( const char* methodname, Executor& ex );
	virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );

};

BApplicObjType datafileelem_type;
class DataFileElemObj
{
public:
	DataFileElemObj( DataFileContentsRef dfcontents, DataFileElementRef dfelem ) :
		dfcontents(dfcontents), dfelem(dfelem) {}
public:
	DataFileContentsRef dfcontents;
	DataFileElementRef dfelem;
};

typedef BApplicObj< DataFileElemObj > DataElemRefObjImpBase;
class DataElemRefObjImp : public DataElemRefObjImpBase
{
public:
	DataElemRefObjImp( DataFileContentsRef dfcontents, DataFileElementRef dflem );
	virtual const char* typeOf() const;
	virtual BObjectImp* copy() const;

	virtual BObjectImp* call_method( const char* methodname, Executor& ex );
	virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
};

class DataStoreFile
{
public:
	explicit DataStoreFile( ConfigElem& elem );
	DataStoreFile( const std::string& descriptor,
				   const Package* pkg,
				   const std::string& name,
				   int flags );
virtual ~DataStoreFile();
	bool loaded() const;
	void load();
	void save() const;
	std::string filename() const;
	std::string filename( unsigned ver ) const;
	void printOn( ostream& os ) const;

	string descriptor;
	string name;

	string pkgname;
	const Package* pkg;
	unsigned version;
	unsigned oldversion;
	int flags;
	bool unload;

	unsigned delversion;

	DataFileContentsRef dfcontents;
};

typedef std::map< string, DataStoreFile*, ci_cmp_pred > DataStore;
DataStore datastore;

template<>
TmplExecutorModule<DataFileExecutorModule>::FunctionDef   
TmplExecutorModule<DataFileExecutorModule>::function_table[] = 
{
	{ "ListDataFiles",			&DataFileExecutorModule::mf_ListDataFiles },
	{ "CreateDataFile",		 &DataFileExecutorModule::mf_CreateDataFile },
	{ "OpenDataFile",		   &DataFileExecutorModule::mf_OpenDataFile },
	{ "UnloadDataFile",		 &DataFileExecutorModule::mf_UnloadDataFile }
};

template<>
int TmplExecutorModule<DataFileExecutorModule>::function_table_size =
arsize(function_table);

DataFileContents::DataFileContents( DataStoreFile* dsf ) :
	dsf(dsf),
	dirty(false)
{
}

DataFileContents::~DataFileContents()
{
	elements_by_integer.clear();
	elements_by_string.clear();
}

void DataFileContents::load( ConfigFile& cf )
{
	ConfigElem elem;

	while (cf.read( elem ))
	{
		DataFileElement* delem = new DataFileElement(elem);
		
		if (dsf->flags & DF_KEYTYPE_INTEGER)
		{
			elements_by_integer[ atol(elem.rest()) ].set(delem);
		}
		else
		{
			elements_by_string[ elem.rest() ].set(delem);
		}
	}
}

void DataFileContents::save( ostream& os )
{
	for( ElementsByString::const_iterator citr = elements_by_string.begin();
		 citr != elements_by_string.end();
		 ++citr )
	{
		os << "Element " << (*citr).first << "\n"
		   << "{\n";
		(*citr).second->printOn( os );
		os << "}\n\n";
	}

	for( ElementsByInteger::const_iterator citr = elements_by_integer.begin();
		 citr != elements_by_integer.end();
		 ++citr )
	{
		os << "Element " << (*citr).first << "\n"
		   << "{\n";
		(*citr).second->printOn( os );
		os << "}\n\n";
	}
}

BObjectImp* DataFileContents::methodCreateElement( int key )
{
	ElementsByInteger::iterator itr = elements_by_integer.find( key );
	DataFileElementRef dfelem;
	if (itr == elements_by_integer.end())
	{
		dfelem.set(new DataFileElement);
		elements_by_integer[key] = dfelem;
		dirty = true;
	}
	else
	{
		dfelem = (*itr).second;
	}
	return new DataElemRefObjImp( DataFileContentsRef(this), dfelem );
}

BObjectImp* DataFileContents::methodCreateElement( const string& key )
{
	ElementsByString::iterator itr = elements_by_string.find( key );
	DataFileElementRef dfelem;
	if (itr == elements_by_string.end())
	{
		dfelem.set(new DataFileElement);
		elements_by_string[key] = dfelem;
		dirty = true;
	}
	else
	{
		dfelem = (*itr).second;
	}
	return new DataElemRefObjImp( DataFileContentsRef(this), dfelem );
}


BObjectImp* DataFileContents::methodFindElement( int key )
{
	ElementsByInteger::iterator itr = elements_by_integer.find( key );
	if (itr != elements_by_integer.end())
	{
		DataFileElementRef dfelem = (*itr).second;
		return new DataElemRefObjImp( DataFileContentsRef(this), dfelem );
	}
	else
	{
		return new BError( "Element not found" );
	}
}

BObjectImp* DataFileContents::methodFindElement( const string& key )
{
	ElementsByString::iterator itr = elements_by_string.find( key );
	if (itr != elements_by_string.end())
	{
		DataFileElementRef dfelem = (*itr).second;
		return new DataElemRefObjImp( DataFileContentsRef(this), dfelem );	
	}
	else
	{
		return new BError( "Element not found" );
	}
}


BObjectImp* DataFileContents::methodDeleteElement( int key )
{
	if (elements_by_integer.erase( key ))
	{
		dirty = true;
		return new BLong(1);
	}
	else
		return new BError( "Element not found" );
}

BObjectImp* DataFileContents::methodDeleteElement( const string& key )
{
	if (elements_by_string.erase( key ))
	{
		dirty = true;
		return new BLong(1);
	}
	else
		return new BError( "Element not found" );
}

BObjectImp* DataFileContents::methodKeys() const
{
	ObjArray* arr = new ObjArray;
	
	for( ElementsByString::const_iterator citr = elements_by_string.begin();
		 citr != elements_by_string.end();
		 ++citr )
	{
		arr->addElement( new String( (*citr).first ) );
	}

	for( ElementsByInteger::const_iterator citr = elements_by_integer.begin();
		 citr != elements_by_integer.end();
		 ++citr )
	{
		arr->addElement( new BLong( (*citr).first ) );
	}

	return arr;
}



DataFileRefObjImp::DataFileRefObjImp( DataFileContentsRef dfcontents ) :
	DataFileRefObjImpBase( &datafileref_type, dfcontents )
{
}

const char* DataFileRefObjImp::typeOf() const
{
	return "DataFileRef";
}

BObjectImp* DataFileRefObjImp::copy() const
{
	return new DataFileRefObjImp( obj_ );
}

BObjectImp* DataFileRefObjImp::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
	switch(id)
	{
	case MTH_CREATEELEMENT:
		if (!ex.hasParams(1))
		{
			return new BError( "not enough parameters to datafile.createelement(key)" );
		}
		if (obj_->dsf->flags & DF_KEYTYPE_INTEGER)
		{
			int key;
			if (!ex.getParam( 0, key ))
			{
				return new BError( "datafile.createelement(key): key must be an Integer" );
			}
			return obj_->methodCreateElement( key );
		}
		else
		{
			const String* key;
			if (!ex.getStringParam( 0, key ))
			{
				return new BError( "datafile.createelement(key): key must be a String" );
			}
			return obj_->methodCreateElement( key->value() );
		}   
		break;
	case MTH_FINDELEMENT:
		if (!ex.hasParams(1))
		{
			return new BError( "not enough parameters to datafile.findelement(key)" );
		}
		if (obj_->dsf->flags & DF_KEYTYPE_INTEGER)
		{
			int key;
			if (!ex.getParam( 0, key ))
			{
				return new BError( "datafile.findelement(key): key must be an Integer" );
			}
			return obj_->methodFindElement( key );
		}
		else
		{
			const String* key;
			if (!ex.getStringParam( 0, key ))
			{
				return new BError( "datafile.findelement(key): key must be a String" );
			}
			return obj_->methodFindElement( key->value() );
		}
		break;
	case MTH_DELETEELEMENT:
		if (!ex.hasParams(1))
		{
			return new BError( "not enough parameters to datafile.deleteelement(key)" );
		}
		if (obj_->dsf->flags & DF_KEYTYPE_INTEGER)
		{
			int key;
			if (!ex.getParam( 0, key ))
			{
				return new BError( "datafile.deleteelement(key): key must be an Integer" );
			}
			return obj_->methodDeleteElement( key );
		}
		else
		{
			const String* key;
			if (!ex.getStringParam( 0, key ))
			{
				return new BError( "datafile.deleteelement(key): key must be a String" );
			}
			return obj_->methodDeleteElement( key->value() );
		}
		break;
	case MTH_KEYS:
		return obj_->methodKeys();
	default:
		return NULL;
	}
}

BObjectImp* DataFileRefObjImp::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return NULL;
	/*
	if (stricmp( methodname, "createelement" ) == 0)
	{
		if (!ex.hasParams(1))
		{
			return new BError( "not enough parameters to datafile.createelement(key)" );
		}
		if (obj_->dsf->flags & DF_KEYTYPE_INTEGER)
		{
			int key;
			if (!ex.getParam( 0, key ))
			{
				return new BError( "datafile.createelement(key): key must be an Integer" );
			}
			return obj_->methodCreateElement( key );
		}
		else
		{
			const String* key;
			if (!ex.getStringParam( 0, key ))
			{
				return new BError( "datafile.createelement(key): key must be a String" );
			}
			return obj_->methodCreateElement( key->value() );
		}
	}
	else if (stricmp( methodname, "findelement" ) == 0)
	{
		if (!ex.hasParams(1))
		{
			return new BError( "not enough parameters to datafile.findelement(key)" );
		}
		if (obj_->dsf->flags & DF_KEYTYPE_INTEGER)
		{
			int key;
			if (!ex.getParam( 0, key ))
			{
				return new BError( "datafile.findelement(key): key must be an Integer" );
			}
			return obj_->methodFindElement( key );
		}
		else
		{
			const String* key;
			if (!ex.getStringParam( 0, key ))
			{
				return new BError( "datafile.findelement(key): key must be a String" );
			}
			return obj_->methodFindElement( key->value() );
		}
	}
	else if (stricmp( methodname, "deleteelement" ) == 0)
	{
		if (!ex.hasParams(1))
		{
			return new BError( "not enough parameters to datafile.deleteelement(key)" );
		}
		if (obj_->dsf->flags & DF_KEYTYPE_INTEGER)
		{
			int key;
			if (!ex.getParam( 0, key ))
			{
				return new BError( "datafile.deleteelement(key): key must be an Integer" );
			}
			return obj_->methodDeleteElement( key );
		}
		else
		{
			const String* key;
			if (!ex.getStringParam( 0, key ))
			{
				return new BError( "datafile.deleteelement(key): key must be a String" );
			}
			return obj_->methodDeleteElement( key->value() );
		}
	}
	else if (stricmp( methodname, "keys" ) == 0)
	{
		return obj_->methodKeys();
	}

	return NULL;
	*/
}



DataElemRefObjImp::DataElemRefObjImp( DataFileContentsRef dfcontents, DataFileElementRef dfelem ) :
	DataElemRefObjImpBase( &datafileelem_type, DataFileElemObj(dfcontents,dfelem) )
{
}
const char* DataElemRefObjImp::typeOf() const
{
	return "DataElemRef";
}
BObjectImp* DataElemRefObjImp::copy() const
{
	return new DataElemRefObjImp( obj_.dfcontents, obj_.dfelem );
}

BObjectImp* DataElemRefObjImp::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
	bool changed = false;
	BObjectImp* res = CallPropertyListMethod_id( obj_.dfelem->proplist, id, ex, changed );
	if (changed)
		obj_.dfcontents->dirty = true;
	return res;
}

BObjectImp* DataElemRefObjImp::call_method( const char* methodname, Executor& ex )
{
	bool changed = false;
	BObjectImp* res = CallPropertyListMethod( obj_.dfelem->proplist, methodname, ex, changed );
	if (changed)
		obj_.dfcontents->dirty = true;
	return res;
}

DataStoreFile* DataFileExecutorModule::GetDataStoreFile( const std::string& inspec )
{
	string descriptor;

	const Package* spec_pkg = NULL;
	string spec_filename;
	if (!pkgdef_split( inspec, exec.prog()->pkg,
					   &spec_pkg, &spec_filename ))
	{
		return NULL; //new BError( "Error in descriptor" );
	}
	if (spec_pkg == NULL)
	{
		// ::filename
		descriptor = "::" + spec_filename;
	}
	else
	{
		// :somepkg:filename
		descriptor = ":" + spec_pkg->name() + ":" + spec_filename;
	}

	DataStore::iterator itr = datastore.find( descriptor );
	if (itr != datastore.end())
	{
		DataStoreFile* dsf = (*itr).second;
		return dsf;
	}
	else
	{
		return NULL;
	}
}

BObjectImp* DataFileExecutorModule::mf_ListDataFiles()
{
	ObjArray* file_list = new ObjArray;
	for( DataStore::iterator itr = datastore.begin(); itr != datastore.end(); ++itr )
	{
		DataStoreFile* dsf = (*itr).second;
		BStruct* file_name = new BStruct;
		file_name->addMember("pkg", new String(dsf->pkgname));
		file_name->addMember("name", new String(dsf->name));
		file_name->addMember("descriptor", new String(dsf->descriptor));

		file_list->addElement(file_name);
	}
	return file_list;
}

BObjectImp* DataFileExecutorModule::mf_CreateDataFile()
{
	const String* strob;
	int flags;
	if (getStringParam( 0, strob ) &&
		getParam( 1, flags ))
	{
		try
		{
			string descriptor;
			string directory;
			string d_ds;
			const string& inspec = strob->value();
		
			const Package* spec_pkg = NULL;
			string spec_filename;
			if (!pkgdef_split( inspec, exec.prog()->pkg,
							   &spec_pkg, &spec_filename ))
			{
				return new BError( "Error in descriptor" );
			}
			if (spec_pkg == NULL)
			{
				// ::filename
				descriptor = "::" + spec_filename;
				directory = config.world_data_path + "ds/";
			}
			else
			{
				// :somepkg:filename
				descriptor = ":" + spec_pkg->name() + ":" + spec_filename;
				d_ds = config.world_data_path + "ds/";
				directory = config.world_data_path + "ds/" + spec_pkg->name() + "/";
			}
			if (!FileExists( directory.c_str() ))
			{
				if (!d_ds.empty())
					MakeDirectory( d_ds.c_str() );
				MakeDirectory( directory.c_str() );
			}

			DataStoreFile* dsf = NULL;

			DataStore::iterator itr = datastore.find( descriptor );
			if (itr != datastore.end())
			{
				dsf = (*itr).second;
			}
			else
			{
				// create a new one
				dsf = new DataStoreFile( descriptor, spec_pkg, spec_filename, flags );
				datastore[ descriptor ] = dsf;
			}

			// didn't find it, time to go open or create.
			if (!dsf->loaded())
				dsf->load();

			return new DataFileRefObjImp( dsf->dfcontents );
		}
		catch(std::exception& ex)
		{
			string message = string("An exception occurred: ") + ex.what();
			return new BError( message );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* DataFileExecutorModule::mf_OpenDataFile()
{
	const String* strob;
	if (getStringParam( 0, strob ))
	{
		try
		{
			string descriptor;
//			string directory;
			const string& inspec = strob->value();
		
			const Package* spec_pkg = NULL;
			string spec_filename;
			if (!pkgdef_split( inspec, exec.prog()->pkg,
							   &spec_pkg, &spec_filename ))
			{
				return new BError( "Error in descriptor" );
			}
			if (spec_pkg == NULL)
			{
				// ::filename
				descriptor = "::" + spec_filename;
			}
			else
			{
				// :somepkg:filename
				descriptor = ":" + spec_pkg->name() + ":" + spec_filename;
			}

			DataStore::iterator itr = datastore.find( descriptor );
			if (itr != datastore.end())
			{
				DataStoreFile* dsf = (*itr).second;
				// didn't find it, time to go open or create.
				if (!dsf->loaded())
					dsf->load();
				return new DataFileRefObjImp( dsf->dfcontents );
			}
			else
			{
				return new BError( "Datafile does not exist" );
			}
		}
		catch(exception& ex)
		{
			return new BError( string("An exception occurred") + ex.what() );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* DataFileExecutorModule::mf_UnloadDataFile()
{
	const String* strob;
	if (getStringParam( 0, strob ))
	{
		DataStoreFile* dsf = GetDataStoreFile( strob->value() );
		if (!dsf)
			return new BError( "Unable to find data store file" );

		dsf->unload = true;
		return new BLong(1);
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

DataStoreFile::DataStoreFile( ConfigElem& elem ) :
	descriptor( elem.remove_string( "Descriptor" ) ),
	name( elem.remove_string( "name" ) ),
	pkgname( elem.remove_string( "package", "" ) ),
	pkg( find_package( pkgname ) ),
	version( elem.remove_ushort( "Version" ) ),
	oldversion( elem.remove_ushort( "OldVersion" ) ),
	flags( elem.remove_ulong( "Flags" )),
	unload(false)
{
}

DataStoreFile::DataStoreFile( const std::string& descriptor,
							  const Package* pkg,
							  const std::string& name,
							  int flags ) :
	descriptor(descriptor),
	name(name),
	pkgname(""),
	pkg(pkg),
	version(0),
	oldversion(0),
	flags(flags),
	unload(false)
{
	if (pkg != NULL)
		pkgname = pkg->name();
}

bool DataStoreFile::loaded() const
{
	return dfcontents.get() != NULL;
}

void DataStoreFile::load()
{
	if (loaded())
		return;

	dfcontents.set( new DataFileContents( this ) );

	string fn = filename();
	if (FileExists( fn.c_str() ))
	{
		ConfigFile cf( filename().c_str(), "Element" );
		dfcontents->load( cf );
	}
	else
	{
		// just force an empty file to be written
		dfcontents->dirty = true;
	}
}

DataStoreFile::~DataStoreFile()
{
	dfcontents.clear();

}

void DataStoreFile::printOn( ostream& os ) const
{
	os << "DataFile\n"
	   << "{\n"
	   << "\tDescriptor\t" << descriptor << "\n"
	   << "\tName\t" << name << "\n";
	
	if (!pkgname.empty())
	   os << "\tPackage\t" << pkgname << "\n";

	os << "\tFlags\t" << flags << "\n"
	   << "\tVersion\t" << version << "\n"
	   << "\tOldVersion\t" << oldversion << "\n"
	   << "}\n\n";
}

std::string DataStoreFile::filename( unsigned ver ) const
{
	string tmp = config.world_data_path + "ds/";
	if (pkg != NULL)
		tmp += pkg->name() + "/";
	tmp += name + "." + tostring(ver%10) + ".txt";
	return tmp;
}

std::string DataStoreFile::filename() const
{
	return filename( version );
}

void DataStoreFile::save() const
{
	string fname = filename();
	ofstream ofs( fname.c_str(), ios::out );
	dfcontents->save( ofs );
}

DataFileElement::DataFileElement() :
	proplist()
{
}

DataFileElement::DataFileElement( ConfigElem& elem )
{
	proplist.readRemainingPropertiesAsStrings( elem );
}

void DataFileElement::printOn( ostream& os ) const
{
	proplist.printPropertiesAsStrings(os);
}

void read_datastore_dat()
{
	string datastorefile = config.world_data_path + "datastore.txt";

	if (!FileExists( datastorefile))
		return;
	
	ConfigFile cf( datastorefile, "DataFile" );
	ConfigElem elem;

	while (cf.read( elem ))
	{
		DataStoreFile* dsf = new DataStoreFile( elem );
		datastore[ dsf->descriptor ] = dsf;
	}
}

void write_datastore( ostream& os )
{
	for( DataStore::iterator itr = datastore.begin(); itr != datastore.end(); ++itr )
	{
		DataStoreFile* dsf = (*itr).second;

		dsf->delversion = dsf->oldversion;
		dsf->oldversion = dsf->version;

		if (dsf->dfcontents.get() && dsf->dfcontents->dirty)
		{
			// make a new generation of file and write it.
			++dsf->version;

			dsf->save();

			dsf->dfcontents->dirty = false;
		}

		dsf->printOn( os );
	}
}

void commit_datastore()
{
	for( DataStore::iterator itr = datastore.begin(); itr != datastore.end(); ++itr )
	{
		DataStoreFile* dsf = (*itr).second;

		if (dsf->delversion != dsf->version &&
			dsf->delversion != dsf->oldversion)
		{
			RemoveFile( dsf->filename(dsf->delversion).c_str() );
		}

		if (dsf->unload)
		{
			if (dsf->dfcontents.get() != NULL)
			{
				if (dsf->dfcontents->count() == 1)
				{
					dsf->dfcontents.clear();
				}
			}
			dsf->unload = false;
		}
	}
}

void unload_datastore()
{
	for( DataStore::iterator itr = datastore.begin(); itr != datastore.end(); ++itr )
	{
		DataStoreFile* dsf = (*itr).second;
		delete dsf;

	}

	datastore.clear();
}

