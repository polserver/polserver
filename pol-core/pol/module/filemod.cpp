/** @file
 *
 * @par History
 * - 2005/04/31 Shinigami: mf_LogToFile - added flag to log Core-Style DateTimeStr in front of log
 * entry
 * - 2006/09/27 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 * - 2009/12/18 Turley:    added CreateDirectory() & ListDirectory()
 * - 2011/01/07 Nando:     mf_LogToFile - check the return of strftime
 */


#include <cerrno>
#include <ctime>
#include <iosfwd>
#include <string>

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/impstr.h"
#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/clib.h"
#include "../../clib/dirlist.h"
#include "../../clib/fileutil.h"
#include "../../clib/logfacility.h"
#include "../../clib/stlutil.h"
#include "../../plib/pkg.h"
#include "../binaryfilescrobj.h"
#include "../core.h"
#include "../globals/ucfg.h"
#include "../xmlfilescrobj.h"
#include "fileaccess.h"
#include "filemod.h"
#include "modules/file-tbl.h"

namespace Pol
{
namespace Module
{
using namespace Bscript;

/**
 * I'm thinking that, if anything, I'd want to present a VERY simple, high-level interface.
 *
 * So no seeking / reading / writing.  Something more like this:
 * - all filenames are package-relative.
 * - Only files in packages can be accessed.
 * - ReadFile( filename ); // returns the entire file as an array of lines
 * - WriteFile( filename, contents ); // writes a file in its entirely
 * - AppendToFile( filename, data );
 * - LogToFile( filename, text );
 *
 * WriteFile and AppendToFile would accept either a string or an array of lines.  The
 * string would be written verbatim, while the array of lines would be written with
 * newlines appended.
 *
 * WriteFile would be atomic for existing files - it would write a new file to a temporary
 * location, then do the rename-rename-delete thing.
 *
 * LogToFile would take a string as input, and append a newline.  It's the equivalent of
 * AppendToFile( filename, array { text } )
 *
 * Probably a systemwide configuration file would dictate the types of files allowed to
 * be written, and perhaps even name the packages allowed to do so.  I could see a
 * use for being able to write .src files, for example.  And I'm having thoughts of
 * integrating compile capability directly into pol, and adding something like
 * CompileScript( scriptpath ) to polsys.em
 *
 * config/fileaccess.cfg
 * ~~~~~~~~~~~~~~~~~~~~~
 * // anyone can append to a log file in their own package
 * FileAccess
 * {
 *   allow append
 *   match *.log
 *   package <all>
 * }
 * // anyone can create .htm files in their own web root
 * FileAccess
 * {
 *   allow write
 *   match www/ *.html
 *   package <all>
 * }
 * // the uploader package is allowed to write .src and .cfg files in any package
 * FileAccess
 * {
 *   allow read
 *   allow write
 *   remote 1
 *   match *.cfg
 *   match *.src
 *   package uploader
 * }
 * ~~~~~~~~~~~~~~~~~~~~~
 */
FileAccess::FileAccess( Clib::ConfigElem& elem )
    : AllowWrite( elem.remove_bool( "AllowWrite", false ) ),
      AllowAppend( elem.remove_bool( "AllowAppend", false ) ),
      AllowRead( elem.remove_bool( "AllowRead", false ) ),
      AllowRemote( elem.remove_bool( "AllowRemote", false ) ),
      AllPackages( false ),
      AllDirectories( false ),
      AllExtensions( false )
{
  std::string tmp;
  while ( elem.remove_prop( "Package", &tmp ) )
  {
    if ( tmp == "*" )
      AllPackages = true;
    else
    {
      auto pkg = Plib::find_package( tmp );
      if ( pkg == nullptr )
        ERROR_PRINT << "Fileaccess package entry not found: " << tmp << "\n";
      else
        Packages.insert( pkg );
    }
    while ( elem.remove_prop( "Directory", &tmp ) )
    {
      if ( tmp == "*" )
        AllDirectories = true;
      else
      {
        const Plib::Package* cfgpkg;
        std::string cfgpath;
        if ( pkgdef_split( tmp, nullptr, &cfgpkg, &cfgpath ) )
          Directories.push_back( std::make_pair( cfgpkg, cfgpath ) );
        else
          ERROR_PRINT << "Invalid fileaccess Directory entry: " << tmp << "\n";
      }
    }
    if ( Directories.empty() )
      AllDirectories = true;
    while ( elem.remove_prop( "Extension", &tmp ) )
    {
      if ( tmp == "*" )
        AllExtensions = true;
      else
        Extensions.push_back( tmp );
    }
  }
}
bool FileAccess::AllowsAccessTo( const Plib::Package* pkg, const Plib::Package* filepackage ) const
{
  if ( AllowRemote )
    return true;

  if ( pkg == filepackage )
    return true;

  return false;
}

bool FileAccess::AppliesToPackage( const Plib::Package* pkg ) const
{
  if ( AllPackages )
    return true;

  if ( pkg == nullptr )
    return false;

  if ( Packages.count( pkg ) )
    return true;

  return false;
}

bool FileAccess::AppliesToPath( const std::string& path, const Plib::Package* filepkg ) const
{
  if ( !AllDirectories )
  {
    std::string filepath = path;
    if ( Clib::strip_one( filepath ) != 0 )
      filepath = "";

    bool found( false );
    for ( const auto& dir : Directories )
    {
      const auto& cfgpkg = dir.first;
      const auto& cfgpath = dir.second;
      if ( cfgpkg == filepkg )
      {
        if ( filepath.size() < cfgpath.size() )
        {
          found = false;
          continue;
        }
        found = true;
        for ( size_t i = 0; i < filepath.size() && i < cfgpath.size(); ++i )
        {
          if ( filepath[i] != cfgpath[i] )
          {
            found = false;
            break;
          }
        }
        if ( found )
          break;
      }
    }
    if ( !found )
      return false;
  }
  if ( AllExtensions )
    return true;

  // check for weirdness in the path
  if ( path.find( '\0' ) != std::string::npos )
    return false;
  if ( path.find( ".." ) != std::string::npos )
    return false;
  for ( const auto& ext : Extensions )
  {
    if ( path.size() >= ext.size() )
    {
      std::string path_ext = path.substr( path.size() - ext.size() );
      if ( Clib::stringicmp( path_ext, ext ) == 0 )
      {
        return true;
      }
    }
  }
  return false;
}

size_t FileAccess::estimateSize() const
{
  size_t size = sizeof( FileAccess );
  size += Packages.size() * ( sizeof( void* ) + 3 * sizeof( void* ) );

  for ( const auto& d : Directories )
    size += sizeof( decltype( Directories )::value_type ) + d.second.capacity();
  for ( const auto& e : Extensions )
    size += e.capacity();
  return size;
}

bool HasReadAccess( const Plib::Package* pkg, const Plib::Package* filepackage,
                    const std::string& path )
{
  for ( const auto& fa : Core::configurationbuffer.file_access_rules )
  {
    if ( fa.AllowRead && fa.AllowsAccessTo( pkg, filepackage ) && fa.AppliesToPackage( pkg ) &&
         fa.AppliesToPath( path, filepackage ) )
    {
      return true;
    }
  }
  return false;
}
bool HasWriteAccess( const Plib::Package* pkg, const Plib::Package* filepackage,
                     const std::string& path )
{
  for ( const auto& fa : Core::configurationbuffer.file_access_rules )
  {
    if ( fa.AllowWrite && fa.AllowsAccessTo( pkg, filepackage ) && fa.AppliesToPackage( pkg ) &&
         fa.AppliesToPath( path, filepackage ) )
    {
      return true;
    }
  }
  return false;
}
bool HasAppendAccess( const Plib::Package* pkg, const Plib::Package* filepackage,
                      const std::string& path )
{
  for ( const auto& fa : Core::configurationbuffer.file_access_rules )
  {
    if ( fa.AllowAppend && fa.AllowsAccessTo( pkg, filepackage ) && fa.AppliesToPackage( pkg ) &&
         fa.AppliesToPath( path, filepackage ) )
    {
      return true;
    }
  }
  return false;
}

FileAccessExecutorModule::FileAccessExecutorModule( Bscript::Executor& exec )
    : TmplExecutorModule<FileAccessExecutorModule>( exec )
{
}

ExecutorModule* CreateFileAccessExecutorModule( Executor& exec )
{
  return new FileAccessExecutorModule( exec );
}

Bscript::BObjectImp* FileAccessExecutorModule::mf_FileExists()
{
  const String* filename;
  if ( !getStringParam( 0, filename ) )
    return new BError( "Invalid parameter type." );

  const Plib::Package* outpkg;
  std::string path;
  if ( !pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ) )
    return new BError( "Error in filename descriptor." );

  if ( path.find( ".." ) != std::string::npos )
    return new BError( "No parent path traversal allowed." );

  std::string filepath;
  if ( outpkg == nullptr )
    filepath = path;
  else
    filepath = outpkg->dir() + path;

  return new BLong( Clib::FileExists( filepath ) );
}

Bscript::BObjectImp* FileAccessExecutorModule::mf_ReadFile()
{
  const String* filename;
  if ( !getStringParam( 0, filename ) )
    return new BError( "Invalid parameter type" );

  const Plib::Package* outpkg;
  std::string path;
  if ( !pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ) )
    return new BError( "Error in filename descriptor" );

  if ( path.find( ".." ) != std::string::npos )
    return new BError( "No parent path traversal please." );

  if ( !HasReadAccess( exec.prog()->pkg, outpkg, path ) )
    return new BError( "Access denied" );

  std::string filepath;
  if ( outpkg == nullptr )
    filepath = path;
  else
    filepath = outpkg->dir() + path;

  std::ifstream ifs( filepath.c_str() );
  if ( !ifs.is_open() )
    return new BError( "File not found: " + filepath );

  std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray() );

  std::string line;
  while ( getline( ifs, line ) )
    arr->addElement( new String( line ) );

  return arr.release();
}

Bscript::BObjectImp* FileAccessExecutorModule::mf_WriteFile()
{
  const String* filename;
  Bscript::ObjArray* contents;
  if ( !getStringParam( 0, filename ) || !getObjArrayParam( 1, contents ) )
  {
    return new BError( "Invalid parameter type" );
  }
  const Plib::Package* outpkg;
  std::string path;
  if ( !pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ) )
    return new BError( "Error in filename descriptor" );

  if ( path.find( ".." ) != std::string::npos )
    return new BError( "No parent path traversal please." );
  if ( !HasWriteAccess( exec.prog()->pkg, outpkg, path ) )
    return new BError( "Access denied" );

  std::string filepath;
  if ( outpkg == nullptr )
    filepath = path;
  else
    filepath = outpkg->dir() + path;

  std::string bakpath = filepath + ".bak";
  std::string tmppath = filepath + ".tmp";

  std::ofstream ofs( tmppath.c_str(), std::ios::out | std::ios::trunc );

  if ( !ofs.is_open() )
    return new BError( "File not found: " + filepath );

  for ( unsigned i = 0; i < contents->ref_arr.size(); ++i )
  {
    BObjectRef& ref = contents->ref_arr[i];
    BObject* obj = ref.get();
    if ( obj != nullptr )
    {
      ofs << ( *obj )->getStringRep();
    }
    ofs << std::endl;
  }
  if ( ofs.fail() )
    return new BError( "Error during write." );
  ofs.close();

  if ( Clib::FileExists( bakpath ) )
  {
    if ( unlink( bakpath.c_str() ) )
    {
      int err = errno;
      std::string message = "Unable to remove " + filepath + ": " + strerror( err );

      return new BError( message );
    }
  }
  if ( Clib::FileExists( filepath ) )
  {
    if ( rename( filepath.c_str(), bakpath.c_str() ) )
    {
      int err = errno;
      std::string message =
          "Unable to rename " + filepath + " to " + bakpath + ": " + strerror( err );
      return new BError( message );
    }
  }
  if ( rename( tmppath.c_str(), filepath.c_str() ) )
  {
    int err = errno;
    std::string message =
        "Unable to rename " + tmppath + " to " + filepath + ": " + strerror( err );
    return new BError( message );
  }

  return new BLong( 1 );
}

Bscript::BObjectImp* FileAccessExecutorModule::mf_AppendToFile()
{
  const String* filename;
  ObjArray* contents;
  if ( !getStringParam( 0, filename ) || !getObjArrayParam( 1, contents ) )
  {
    return new BError( "Invalid parameter type" );
  }

  const Plib::Package* outpkg;
  std::string path;
  if ( !pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ) )
    return new BError( "Error in filename descriptor" );

  if ( path.find( ".." ) != std::string::npos )
    return new BError( "No parent path traversal please." );

  if ( !HasAppendAccess( exec.prog()->pkg, outpkg, path ) )
    return new BError( "Access denied" );

  std::string filepath;
  if ( outpkg == nullptr )
    filepath = path;
  else
    filepath = outpkg->dir() + path;

  std::ofstream ofs( filepath.c_str(), std::ios::out | std::ios::app );

  if ( !ofs.is_open() )
    return new BError( "Unable to open file: " + filepath );

  for ( unsigned i = 0; i < contents->ref_arr.size(); ++i )
  {
    BObjectRef& ref = contents->ref_arr[i];
    BObject* obj = ref.get();
    if ( obj != nullptr )
    {
      ofs << ( *obj )->getStringRep();
    }
    ofs << std::endl;
  }
  if ( ofs.fail() )
    return new BError( "Error during write." );

  return new BLong( 1 );
}

Bscript::BObjectImp* FileAccessExecutorModule::mf_LogToFile()
{  //
  const String* filename;
  const String* textline;
  if ( getStringParam( 0, filename ) && getStringParam( 1, textline ) )
  {
    int flags;
    if ( exec.fparams.size() >= 3 )
    {
      if ( !getParam( 2, flags ) )
        return new BError( "Invalid parameter type" );
    }
    else
    {
      flags = 0;
    }

    const Plib::Package* outpkg;
    std::string path;
    if ( !pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ) )
      return new BError( "Error in filename descriptor" );

    if ( path.find( ".." ) != std::string::npos )
      return new BError( "No parent path traversal please." );

    if ( !HasAppendAccess( exec.prog()->pkg, outpkg, path ) )
      return new BError( "Access denied" );

    std::string filepath;
    if ( outpkg == nullptr )
      filepath = path;
    else
      filepath = outpkg->dir() + path;

    std::ofstream ofs( filepath.c_str(), std::ios::out | std::ios::app );

    if ( !ofs.is_open() )
      return new BError( "Unable to open file: " + filepath );

    if ( flags & Core::LOG_DATETIME )
    {
      auto time_tm = Clib::localtime( time( nullptr ) );

      char buffer[30];
      if ( strftime( buffer, sizeof buffer, "%m/%d %H:%M:%S", &time_tm ) > 0 )
        ofs << "[" << buffer << "] ";
    }

    ofs << textline->value() << std::endl;

    if ( ofs.fail() )
      return new BError( "Error during write." );

    return new BLong( 1 );
  }
  else
    return new BError( "Invalid parameter type" );
}

Bscript::BObjectImp* FileAccessExecutorModule::mf_OpenBinaryFile()
{
  const String* filename;
  unsigned short mode, bigendian;
  if ( ( !getStringParam( 0, filename ) ) || ( !getParam( 1, mode ) ) ||
       ( !getParam( 2, bigendian ) ) )
    return new BError( "Invalid parameter type" );

  const Plib::Package* outpkg;
  std::string path;
  if ( !pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ) )
    return new BError( "Error in filename descriptor" );

  if ( path.find( ".." ) != std::string::npos )
    return new BError( "No parent path traversal please." );

  if ( mode & 0x01 )
  {
    if ( !HasReadAccess( exec.prog()->pkg, outpkg, path ) )
      return new BError( "Access denied" );
  }
  if ( mode & 0x02 )
  {
    if ( !HasWriteAccess( exec.prog()->pkg, outpkg, path ) )
      return new BError( "Access denied" );
  }

  std::string filepath;
  if ( outpkg == nullptr )
    filepath = path;
  else
    filepath = outpkg->dir() + path;

  return new Core::BBinaryfile( filepath, mode, bigendian == 1 ? true : false );
}

Bscript::BObjectImp* FileAccessExecutorModule::mf_CreateDirectory()
{
  const String* dirname;
  if ( !getStringParam( 0, dirname ) )
    return new BError( "Invalid parameter type" );

  const Plib::Package* outpkg;
  std::string path;
  if ( !pkgdef_split( dirname->value(), exec.prog()->pkg, &outpkg, &path ) )
    return new BError( "Error in dirname descriptor" );
  if ( path.find( ".." ) != std::string::npos )
    return new BError( "No parent path traversal please." );

  if ( outpkg != nullptr )
    path = outpkg->dir() + path;
  path = Clib::normalized_dir_form( path );
  if ( Clib::IsDirectory( path.c_str() ) )
    return new BError( "Directory already exists." );
  int res = Clib::make_dir( path.c_str() );
  if ( res != 0 )
    return new BError( "Could not create directory." );
  return new BLong( 1 );
}

Bscript::BObjectImp* FileAccessExecutorModule::mf_ListDirectory()
{
  const String* dirname;
  const String* extension;
  short listdirs;
  if ( ( !getStringParam( 0, dirname ) ) || ( !getStringParam( 1, extension ) ) ||
       ( !getParam( 2, listdirs ) ) )
    return new BError( "Invalid parameter type" );

  const Plib::Package* outpkg;
  std::string path;
  if ( !pkgdef_split( dirname->value(), exec.prog()->pkg, &outpkg, &path ) )
    return new BError( "Error in dirname descriptor" );
  if ( path.find( ".." ) != std::string::npos )
    return new BError( "No parent path traversal please." );

  if ( outpkg != nullptr )
    path = outpkg->dir() + path;
  path = Clib::normalized_dir_form( path );
  if ( !Clib::IsDirectory( path.c_str() ) )
    return new BError( "Directory not found." );
  bool asterisk = false;
  bool nofiles = false;
  if ( extension->getStringRep().find( '*', 0 ) != std::string::npos )
    asterisk = true;
  else if ( extension->length() == 0 )
    nofiles = true;

  Bscript::ObjArray* arr = new Bscript::ObjArray;

  for ( Clib::DirList dl( path.c_str() ); !dl.at_end(); dl.next() )
  {
    std::string name = dl.name();
    if ( name[0] == '.' )
      continue;

    if ( Clib::IsDirectory( ( path + name ).c_str() ) )
    {
      if ( listdirs == 0 )
        continue;
    }
    else if ( nofiles )
      continue;
    else if ( !asterisk )
    {
      std::string::size_type extensionPointPos = name.rfind( '.' );
      if ( extensionPointPos == std::string::npos )
        continue;
      if ( name.substr( extensionPointPos + 1 ) != extension->value() )
        continue;
    }

    arr->addElement( new String( name ) );
  }

  return arr;
}

Bscript::BObjectImp* FileAccessExecutorModule::mf_OpenXMLFile()
{
  const String* filename;
  if ( !getStringParam( 0, filename ) )
    return new BError( "Invalid parameter type" );

  const Plib::Package* outpkg;
  std::string path;
  if ( !pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ) )
    return new BError( "Error in filename descriptor" );

  if ( path.find( ".." ) != std::string::npos )
    return new BError( "No parent path traversal please." );

  if ( !HasReadAccess( exec.prog()->pkg, outpkg, path ) )
    return new BError( "Access denied" );

  std::string filepath;
  if ( outpkg == nullptr )
    filepath = path;
  else
    filepath = outpkg->dir() + path;

  return new Core::BXMLfile( filepath );
}

Bscript::BObjectImp* FileAccessExecutorModule::mf_CreateXMLFile()
{
  return new Core::BXMLfile();
}

void load_fileaccess_cfg()
{
  Core::configurationbuffer.file_access_rules.clear();

  if ( !Clib::FileExists( "config/fileaccess.cfg" ) )
    return;

  Clib::ConfigFile cf( "config/fileaccess.cfg", "FileAccess" );
  Clib::ConfigElem elem;


  while ( cf.read( elem ) )
  {
    FileAccess fa( elem );
    Core::configurationbuffer.file_access_rules.push_back( fa );
  }
}
}  // namespace Module
}  // namespace Pol
