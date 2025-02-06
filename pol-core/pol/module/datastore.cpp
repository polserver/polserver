/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2006/09/26 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 * - 2007/06/17 Shinigami: added config.world_data_path
 * - 2009/12/21 Turley:    ._method() call fix
 */


#include "datastore.h"
#include <exception>
#include <fstream>
#include <stddef.h>

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/bstruct.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"
#include "../../bscript/objmethods.h"
#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/fileutil.h"
#include "../../clib/rawtypes.h"
#include "../../clib/stlutil.h"
#include "../../clib/streamsaver.h"
#include "../../plib/pkg.h"
#include "../../plib/systemstate.h"
#include "../globals/ucfg.h"
#include "../proplist.h"
#include "datastoreimp.h"

#include <module_defs/datafile.h>

namespace Pol
{
namespace Module
{
///
/// Datastore
///
///  datastore files are stored in
///     config.world_data_path + ds/fname.txt
///     config.world_data_path + ds/{pkgname}/fname.txt
///

Bscript::BApplicObjType datafileref_type;
Bscript::BApplicObjType datafileelem_type;

DataFileContents::DataFileContents( DataStoreFile* dsf ) : dsf( dsf ), dirty( false ) {}

DataFileContents::~DataFileContents()
{
  elements_by_integer.clear();
  elements_by_string.clear();
}

size_t DataFileContents::estimateSize() const
{
  size_t size = sizeof( DataStoreFile* ) /*dsf*/
                + sizeof( bool );        /*dirty*/

  size += Clib::memsize( elements_by_string );
  for ( const auto& ele : elements_by_string )
  {
    if ( ele.second.get() != nullptr )
      size += ele.second->proplist.estimatedSize();
  }
  size += Clib::memsize( elements_by_integer );
  return size;
}

void DataFileContents::load( Clib::ConfigFile& cf )
{
  Clib::ConfigElem elem;

  while ( cf.read( elem ) )
  {
    DataFileElement* delem = new DataFileElement( elem );

    if ( dsf->flags & DF_KEYTYPE_INTEGER )
    {
      elements_by_integer[atol( elem.rest() )].set( delem );
    }
    else
    {
      elements_by_string[elem.rest()].set( delem );
    }
  }
}

void DataFileContents::save( Clib::StreamWriter& sw )
{
  for ( const auto& element : elements_by_string )
  {
    sw.begin( "Element", element.first );
    element.second->printOn( sw );
    sw.end();
  }

  for ( const auto& element : elements_by_integer )
  {
    sw.begin( "Element", element.first );
    element.second->printOn( sw );
    sw.end();
  }
}

Bscript::BObjectImp* DataFileContents::methodCreateElement( int key )
{
  ElementsByInteger::iterator itr = elements_by_integer.find( key );
  DataFileElementRef dfelem;
  if ( itr == elements_by_integer.end() )
  {
    dfelem.set( new DataFileElement );
    elements_by_integer[key] = dfelem;
    dirty = true;
  }
  else
  {
    dfelem = ( *itr ).second;
  }
  return new DataElemRefObjImp( DataFileContentsRef( this ), dfelem );
}

Bscript::BObjectImp* DataFileContents::methodCreateElement( const std::string& key )
{
  ElementsByString::iterator itr = elements_by_string.find( key );
  DataFileElementRef dfelem;
  if ( itr == elements_by_string.end() )
  {
    dfelem.set( new DataFileElement );
    elements_by_string[key] = dfelem;
    dirty = true;
  }
  else
  {
    dfelem = ( *itr ).second;
  }
  return new DataElemRefObjImp( DataFileContentsRef( this ), dfelem );
}


Bscript::BObjectImp* DataFileContents::methodFindElement( int key )
{
  ElementsByInteger::iterator itr = elements_by_integer.find( key );
  if ( itr != elements_by_integer.end() )
  {
    DataFileElementRef dfelem = ( *itr ).second;
    return new DataElemRefObjImp( DataFileContentsRef( this ), dfelem );
  }
  else
  {
    return new Bscript::BError( "Element not found" );
  }
}

Bscript::BObjectImp* DataFileContents::methodFindElement( const std::string& key )
{
  ElementsByString::iterator itr = elements_by_string.find( key );
  if ( itr != elements_by_string.end() )
  {
    DataFileElementRef dfelem = ( *itr ).second;
    return new DataElemRefObjImp( DataFileContentsRef( this ), dfelem );
  }
  else
  {
    return new Bscript::BError( "Element not found" );
  }
}


Bscript::BObjectImp* DataFileContents::methodDeleteElement( int key )
{
  if ( elements_by_integer.erase( key ) )
  {
    dirty = true;
    return new Bscript::BLong( 1 );
  }
  else
    return new Bscript::BError( "Element not found" );
}

Bscript::BObjectImp* DataFileContents::methodDeleteElement( const std::string& key )
{
  if ( elements_by_string.erase( key ) )
  {
    dirty = true;
    return new Bscript::BLong( 1 );
  }
  else
    return new Bscript::BError( "Element not found" );
}

Bscript::BObjectImp* DataFileContents::methodKeys() const
{
  std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );

  for ( ElementsByString::const_iterator citr = elements_by_string.begin();
        citr != elements_by_string.end(); ++citr )
  {
    arr->addElement( new Bscript::String( ( *citr ).first ) );
  }

  for ( ElementsByInteger::const_iterator citr = elements_by_integer.begin();
        citr != elements_by_integer.end(); ++citr )
  {
    arr->addElement( new Bscript::BLong( ( *citr ).first ) );
  }

  return arr.release();
}


DataFileRefObjImp::DataFileRefObjImp( DataFileContentsRef dfcontents )
    : DataFileRefObjImpBase( &datafileref_type, dfcontents )
{
}

const char* DataFileRefObjImp::typeOf() const
{
  return "DataFileRef";
}
u8 DataFileRefObjImp::typeOfInt() const
{
  return OTDataFileRef;
}

Bscript::BObjectImp* DataFileRefObjImp::copy() const
{
  return new DataFileRefObjImp( obj_ );
}

Bscript::BObjectImp* DataFileRefObjImp::call_method_id( const int id, Bscript::Executor& ex,
                                                        bool /*forcebuiltin*/ )
{
  switch ( id )
  {
  case Bscript::MTH_CREATEELEMENT:
    if ( !ex.hasParams( 1 ) )
    {
      return new Bscript::BError( "not enough parameters to datafile.createelement(key)" );
    }
    if ( obj_->dsf->flags & DF_KEYTYPE_INTEGER )
    {
      int key;
      if ( !ex.getParam( 0, key ) )
      {
        return new Bscript::BError( "datafile.createelement(key): key must be an Integer" );
      }
      return obj_->methodCreateElement( key );
    }
    else
    {
      const Bscript::String* key;
      if ( !ex.getStringParam( 0, key ) )
      {
        return new Bscript::BError( "datafile.createelement(key): key must be a String" );
      }
      return obj_->methodCreateElement( key->value() );
    }
    break;
  case Bscript::MTH_FINDELEMENT:
    if ( !ex.hasParams( 1 ) )
    {
      return new Bscript::BError( "not enough parameters to datafile.findelement(key)" );
    }
    if ( obj_->dsf->flags & DF_KEYTYPE_INTEGER )
    {
      int key;
      if ( !ex.getParam( 0, key ) )
      {
        return new Bscript::BError( "datafile.findelement(key): key must be an Integer" );
      }
      return obj_->methodFindElement( key );
    }
    else
    {
      const Bscript::String* key;
      if ( !ex.getStringParam( 0, key ) )
      {
        return new Bscript::BError( "datafile.findelement(key): key must be a String" );
      }
      return obj_->methodFindElement( key->value() );
    }
    break;
  case Bscript::MTH_DELETEELEMENT:
    if ( !ex.hasParams( 1 ) )
    {
      return new Bscript::BError( "not enough parameters to datafile.deleteelement(key)" );
    }
    if ( obj_->dsf->flags & DF_KEYTYPE_INTEGER )
    {
      int key;
      if ( !ex.getParam( 0, key ) )
      {
        return new Bscript::BError( "datafile.deleteelement(key): key must be an Integer" );
      }
      return obj_->methodDeleteElement( key );
    }
    else
    {
      const Bscript::String* key;
      if ( !ex.getStringParam( 0, key ) )
      {
        return new Bscript::BError( "datafile.deleteelement(key): key must be a String" );
      }
      return obj_->methodDeleteElement( key->value() );
    }
    break;
  case Bscript::MTH_KEYS:
    return obj_->methodKeys();
  default:
    return nullptr;
  }
}

Bscript::BObjectImp* DataFileRefObjImp::call_method( const char* methodname, Bscript::Executor& ex )
{
  Bscript::ObjMethod* objmethod = Bscript::getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_method_id( objmethod->id, ex );
  else
    return nullptr;
}


DataElemRefObjImp::DataElemRefObjImp( DataFileContentsRef dfcontents, DataFileElementRef dfelem )
    : DataElemRefObjImpBase( &datafileelem_type, DataFileElemObj( dfcontents, dfelem ) )
{
}
const char* DataElemRefObjImp::typeOf() const
{
  return "DataElemRef";
}
u8 DataElemRefObjImp::typeOfInt() const
{
  return OTDataElemRef;
}
Bscript::BObjectImp* DataElemRefObjImp::copy() const
{
  return new DataElemRefObjImp( obj_.dfcontents, obj_.dfelem );
}

Bscript::BObjectImp* DataElemRefObjImp::call_method_id( const int id, Bscript::Executor& ex,
                                                        bool /*forcebuiltin*/ )
{
  bool changed = false;
  Bscript::BObjectImp* res = CallPropertyListMethod_id( obj_.dfelem->proplist, id, ex, changed );
  if ( changed )
    obj_.dfcontents->dirty = true;
  return res;
}

Bscript::BObjectImp* DataElemRefObjImp::call_method( const char* methodname, Bscript::Executor& ex )
{
  bool changed = false;
  Bscript::BObjectImp* res =
      CallPropertyListMethod( obj_.dfelem->proplist, methodname, ex, changed );
  if ( changed )
    obj_.dfcontents->dirty = true;
  return res;
}

DataFileExecutorModule::DataFileExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<DataFileExecutorModule, Bscript::ExecutorModule>( exec )
{
}

DataStoreFile* DataFileExecutorModule::GetDataStoreFile( const std::string& inspec )
{
  std::string descriptor;

  const Plib::Package* spec_pkg = nullptr;
  std::string spec_filename;
  if ( !Plib::pkgdef_split( inspec, exec.prog()->pkg, &spec_pkg, &spec_filename ) )
  {
    return nullptr;  // new BError( "Error in descriptor" );
  }
  if ( spec_pkg == nullptr )
  {
    // ::filename
    descriptor = "::" + spec_filename;
  }
  else
  {
    // :somepkg:filename
    descriptor = ":" + spec_pkg->name() + ":" + spec_filename;
  }

  Core::DataStore::iterator itr = Core::configurationbuffer.datastore.find( descriptor );
  if ( itr != Core::configurationbuffer.datastore.end() )
  {
    DataStoreFile* dsf = ( *itr ).second;
    return dsf;
  }
  else
  {
    return nullptr;
  }
}

Bscript::BObjectImp* DataFileExecutorModule::mf_ListDataFiles()
{
  std::unique_ptr<Bscript::ObjArray> file_list( new Bscript::ObjArray );
  for ( Core::DataStore::iterator itr = Core::configurationbuffer.datastore.begin();
        itr != Core::configurationbuffer.datastore.end(); ++itr )
  {
    DataStoreFile* dsf = ( *itr ).second;
    std::unique_ptr<Bscript::BStruct> file_name( new Bscript::BStruct );
    file_name->addMember( "pkg", new Bscript::String( dsf->pkgname ) );
    file_name->addMember( "name", new Bscript::String( dsf->name ) );
    file_name->addMember( "descriptor", new Bscript::String( dsf->descriptor ) );

    file_list->addElement( file_name.release() );
  }
  return file_list.release();
}

Bscript::BObjectImp* DataFileExecutorModule::mf_CreateDataFile()
{
  const Bscript::String* strob;
  int flags;
  if ( getStringParam( 0, strob ) && getParam( 1, flags ) )
  {
    try
    {
      std::string descriptor;
      std::string directory;
      std::string d_ds;
      const std::string& inspec = strob->value();

      const Plib::Package* spec_pkg = nullptr;
      std::string spec_filename;
      if ( !Plib::pkgdef_split( inspec, exec.prog()->pkg, &spec_pkg, &spec_filename ) )
      {
        return new Bscript::BError( "Error in descriptor" );
      }
      if ( spec_pkg == nullptr )
      {
        // ::filename
        descriptor = "::" + spec_filename;
        directory = Plib::systemstate.config.world_data_path + "ds/";
      }
      else
      {
        // :somepkg:filename
        descriptor = ":" + spec_pkg->name() + ":" + spec_filename;
        d_ds = Plib::systemstate.config.world_data_path + "ds/";
        directory = Plib::systemstate.config.world_data_path + "ds/" + spec_pkg->name() + "/";
      }
      if ( !Clib::FileExists( directory.c_str() ) )
      {
        if ( !d_ds.empty() )
          Clib::MakeDirectory( d_ds.c_str() );
        Clib::MakeDirectory( directory.c_str() );
      }

      DataStoreFile* dsf = nullptr;

      Core::DataStore::iterator itr = Core::configurationbuffer.datastore.find( descriptor );
      if ( itr != Core::configurationbuffer.datastore.end() )
      {
        dsf = ( *itr ).second;
      }
      else
      {
        // create a new one
        dsf = new DataStoreFile( descriptor, spec_pkg, spec_filename, flags );
        Core::configurationbuffer.datastore[descriptor] = dsf;
      }

      // didn't find it, time to go open or create.
      if ( !dsf->loaded() )
        dsf->load();

      return new DataFileRefObjImp( dsf->dfcontents );
    }
    catch ( std::exception& ex )
    {
      std::string message = std::string( "An exception occurred: " ) + ex.what();
      return new Bscript::BError( message );
    }
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* DataFileExecutorModule::mf_OpenDataFile()
{
  const Bscript::String* strob;
  if ( getStringParam( 0, strob ) )
  {
    try
    {
      std::string descriptor;
      //      string directory;
      const std::string& inspec = strob->value();

      const Plib::Package* spec_pkg = nullptr;
      std::string spec_filename;
      if ( !Plib::pkgdef_split( inspec, exec.prog()->pkg, &spec_pkg, &spec_filename ) )
      {
        return new Bscript::BError( "Error in descriptor" );
      }
      if ( spec_pkg == nullptr )
      {
        // ::filename
        descriptor = "::" + spec_filename;
      }
      else
      {
        // :somepkg:filename
        descriptor = ":" + spec_pkg->name() + ":" + spec_filename;
      }

      Core::DataStore::iterator itr = Core::configurationbuffer.datastore.find( descriptor );
      if ( itr != Core::configurationbuffer.datastore.end() )
      {
        DataStoreFile* dsf = ( *itr ).second;
        // didn't find it, time to go open or create.
        if ( !dsf->loaded() )
          dsf->load();
        return new DataFileRefObjImp( dsf->dfcontents );
      }
      else
      {
        return new Bscript::BError( "Datafile does not exist" );
      }
    }
    catch ( std::exception& ex )
    {
      return new Bscript::BError( std::string( "An exception occurred" ) + ex.what() );
    }
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* DataFileExecutorModule::mf_UnloadDataFile()
{
  const Bscript::String* strob;
  if ( getStringParam( 0, strob ) )
  {
    DataStoreFile* dsf = GetDataStoreFile( strob->value() );
    if ( !dsf )
      return new Bscript::BError( "Unable to find data store file" );

    dsf->unload = true;
    return new Bscript::BLong( 1 );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

DataStoreFile::DataStoreFile( Clib::ConfigElem& elem )
    : descriptor( elem.remove_string( "Descriptor" ) ),
      name( elem.remove_string( "name" ) ),
      pkgname( elem.remove_string( "package", "" ) ),
      pkg( Plib::find_package( pkgname ) ),
      version( elem.remove_ushort( "Version" ) ),
      oldversion( elem.remove_ushort( "OldVersion" ) ),
      flags( elem.remove_ulong( "Flags" ) ),
      unload( false ),
      delversion( 0 )
{
}

DataStoreFile::DataStoreFile( const std::string& descriptor, const Plib::Package* pkg,
                              const std::string& name, int flags )
    : descriptor( descriptor ),
      name( name ),
      pkgname( "" ),
      pkg( pkg ),
      version( 0 ),
      oldversion( 0 ),
      flags( flags ),
      unload( false ),
      delversion( 0 )
{
  if ( pkg != nullptr )
    pkgname = pkg->name();
}

bool DataStoreFile::loaded() const
{
  return dfcontents.get() != nullptr;
}

void DataStoreFile::load()
{
  if ( loaded() )
    return;

  dfcontents.set( new DataFileContents( this ) );

  std::string fn = filename();
  if ( Clib::FileExists( fn.c_str() ) )
  {
    Clib::ConfigFile cf( filename().c_str(), "Element" );
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

void DataStoreFile::printOn( Clib::StreamWriter& sw ) const
{
  sw.begin( "DataFile" );
  sw.add( "Descriptor", descriptor );
  sw.add( "Name", name );

  if ( !pkgname.empty() )
    sw.add( "Package", pkgname );

  sw.add( "Flags", flags );
  sw.add( "Version", version );
  sw.add( "OldVersion", oldversion );
  sw.end();
}

std::string DataStoreFile::filename( unsigned ver ) const
{
  std::string tmp = Plib::systemstate.config.world_data_path + "ds/";
  if ( pkg != nullptr )
    tmp += pkg->name() + "/";
  tmp += name + "." + Clib::tostring( ver % 10 ) + ".txt";
  return tmp;
}

std::string DataStoreFile::filename() const
{
  return filename( version );
}

void DataStoreFile::save() const
{
  Clib::StreamWriter sw( filename() );
  dfcontents->save( sw );
}

size_t DataStoreFile::estimateSize() const
{
  size_t size = descriptor.capacity() + name.capacity() + pkgname.capacity() +
                sizeof( Plib::Package* ) /*pkg*/
                + 3 * sizeof( unsigned ) /*version oldversion delversion*/
                + sizeof( int )          /*flags*/
                + sizeof( bool )         /*unload*/
                + sizeof( DataFileContentsRef );
  if ( dfcontents.get() )
    size += dfcontents->estimateSize();
  return size;
}


DataFileElement::DataFileElement() : proplist( Core::CPropProfiler::Type::DATAFILEELEMENT ) {}

DataFileElement::DataFileElement( Clib::ConfigElem& elem )
    : proplist( Core::CPropProfiler::Type::DATAFILEELEMENT )
{
  proplist.readRemainingPropertiesAsStrings( elem );
}

void DataFileElement::printOn( Clib::StreamWriter& sw ) const
{
  proplist.printPropertiesAsStrings( sw );
}

void read_datastore_dat()
{
  std::string datastorefile = Plib::systemstate.config.world_data_path + "datastore.txt";

  if ( !Clib::FileExists( datastorefile ) )
    return;

  Clib::ConfigFile cf( datastorefile, "DataFile" );
  Clib::ConfigElem elem;

  while ( cf.read( elem ) )
  {
    DataStoreFile* dsf = new DataStoreFile( elem );
    Core::configurationbuffer.datastore[dsf->descriptor] = dsf;
  }
}

void write_datastore( Clib::StreamWriter& sw )
{
  for ( Core::DataStore::iterator itr = Core::configurationbuffer.datastore.begin();
        itr != Core::configurationbuffer.datastore.end(); ++itr )
  {
    DataStoreFile* dsf = ( *itr ).second;

    dsf->delversion = dsf->oldversion;
    dsf->oldversion = dsf->version;

    if ( dsf->dfcontents.get() && dsf->dfcontents->dirty )
    {
      // make a new generation of file and write it.
      ++dsf->version;

      dsf->save();

      dsf->dfcontents->dirty = false;
    }

    dsf->printOn( sw );
  }
}

void commit_datastore()
{
  for ( Core::DataStore::iterator itr = Core::configurationbuffer.datastore.begin();
        itr != Core::configurationbuffer.datastore.end(); ++itr )
  {
    DataStoreFile* dsf = ( *itr ).second;

    if ( dsf->delversion != dsf->version && dsf->delversion != dsf->oldversion )
    {
      Clib::RemoveFile( dsf->filename( dsf->delversion ) );
    }

    if ( dsf->unload )
    {
      if ( dsf->dfcontents.get() != nullptr )
      {
        if ( dsf->dfcontents->count() == 1 )
        {
          dsf->dfcontents.clear();
        }
      }
      dsf->unload = false;
    }
  }
}
}  // namespace Module
}  // namespace Pol
