/*
History
=======
2005/04/31 Shinigami: mf_LogToFile - added flag to log Core-Style DateTimeStr in front of log entry
2006/09/27 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
2009/12/18 Turley:    added CreateDirectory() & ListDirectory()

Notes
=======

*/


#include "../../clib/stl_inc.h"

#include <errno.h>
#include <time.h>

#ifdef __unix__
#	include <unistd.h>
#endif

#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/dirlist.h"
#include "../../clib/fileutil.h"
#include "../../clib/maputil.h"
#include "../../clib/stlutil.h"

#include "../../bscript/berror.h"
#include "../../bscript/execmodl.h"
#include "../../bscript/impstr.h"

#include "../../plib/pkg.h"

#include "../core.h"
#include "filemod.h"
#include "../binaryfilescrobj.h"
#include "../xmlfilescrobj.h"

/*
I'm thinking that, if anything, I'd want to present a VERY simple, high-level interface.  
So no seeking / reading / writing.  Something more like this:

// all filenames are package-relative. 
// Only files in packages can be accessed.  
ReadFile( filename ); // returns the entire file as an array of lines
WriteFile( filename, contents ); // writes a file in its entirely
AppendToFile( filename, data );
LogToFile( filename, text );

WriteFile and AppendToFile would accept either a string or an array of lines.  The 
string would be written verbatim, while the array of lines would be written with 
newlines appended.

WriteFile would be atomic for existing files - it would write a new file to a temporary 
location, then do the rename-rename-delete thing.

LogToFile would take a string as input, and append a newline.  It's the equivalent of 
AppendToFile( filename, array { text } )

Probably a systemwide configuration file would dictate the types of files allowed to 
be written, and perhaps even name the packages allowed to do so.  I could see a 
use for being able to write .src files, for example.  And I'm having thoughts of 
integrating compile capability directly into pol, and adding something like 
CompileScript( scriptpath ) to polsys.em

config/fileaccess.cfg
// anyone can append to a log file in their own package
FileAccess
{
	  allow append
	  match *.log
	  package <all>
}
// anyone can create .htm files in their own web root
FileAccess
{
	  allow write
	  match www/ *.html
	  package <all>
}
// the uploader package is allowed to write .src and .cfg files in any package
FileAccess
{
	  allow read
	  allow write
	  remote 1
	  match *.cfg
	  match *.src
	  package uploader
}

*/

template<>
TmplExecutorModule<FileAccessExecutorModule>::FunctionDef   
TmplExecutorModule<FileAccessExecutorModule>::function_table[] = 
{
	{ "FileExists",		&FileAccessExecutorModule::mf_FileExists },
	{ "ReadFile",		&FileAccessExecutorModule::mf_ReadFile },
	{ "WriteFile",		&FileAccessExecutorModule::mf_WriteFile },
	{ "AppendToFile",	&FileAccessExecutorModule::mf_AppendToFile },
	{ "LogToFile",		&FileAccessExecutorModule::mf_LogToFile },
	{ "OpenBinaryFile", &FileAccessExecutorModule::mf_OpenBinaryFile },
	{ "CreateDirectory",&FileAccessExecutorModule::mf_CreateDirectory },
	{ "ListDirectory",  &FileAccessExecutorModule::mf_ListDirectory },
	{ "OpenXMLFile",    &FileAccessExecutorModule::mf_OpenXMLFile }
};

template<>
int TmplExecutorModule<FileAccessExecutorModule>::function_table_size =
arsize(function_table);

FileAccess::FileAccess( ConfigElem& elem ) :
	AllowWrite( elem.remove_bool( "AllowWrite", false ) ),
	AllowAppend( elem.remove_bool( "AllowAppend", false ) ),
	AllowRead( elem.remove_bool( "AllowRead", false ) ),
	AllowRemote( elem.remove_bool( "AllowRemote", false ) ),
	AllPackages( false ),
	AllDirectories( false ),
	AllExtensions( false )
{
	string tmp;
	while (elem.remove_prop( "Package", &tmp ))
	{
		if (tmp == "*")
			AllPackages = true;
		else
			Packages.insert( tmp );
	}
	while (elem.remove_prop( "Directory", &tmp ))
	{
		if (tmp == "*")
			AllDirectories = true;
		else
			Directories.push_back( tmp );
	}
	while (elem.remove_prop( "Extension", &tmp ))
	{
		if (tmp == "*")
			AllExtensions = true;
		else
			Extensions.push_back( tmp );
	}
}

bool FileAccess::AllowsAccessTo( const Package* pkg, const Package* filepackage ) const
{
	if (AllowRemote)
		return true;

	if (pkg == filepackage)
		return true;

	return false;
}

bool FileAccess::AppliesToPackage( const Package* pkg ) const
{
	if (AllPackages)
		return true;

	if( pkg == NULL )
		return false;
	
	if (Packages.count( pkg->name() ))
		return true;

	return false;
}

bool FileAccess::AppliesToPath( const string& path ) const
{
	if (AllExtensions)
		return true;

	// check for weirdness in the path
	for( unsigned i = 0; i < path.size(); ++i )
	{
		char ch = path[i];
		if (ch == '\0')
			return false;
	}
	if (path.find( ".." ) != string::npos)
		return false;

	for( unsigned i = 0; i < Extensions.size(); ++i )
	{
		string ext = Extensions[i];
		if (path.size() >= ext.size())
		{
			string path_ext = path.substr( path.size()-ext.size() );
			if (stringicmp( path_ext, ext ) == 0)
			{
				return true;
			}
		}
	}
	return false;
}

vector<FileAccess> file_access_rules;

bool HasReadAccess( const Package* pkg, const Package* filepackage, const string& path )
{
	for( unsigned i = 0; i < file_access_rules.size(); ++i )
	{
		const FileAccess& fa = file_access_rules[i];
		if (fa.AllowRead &&
			fa.AllowsAccessTo( pkg, filepackage ) &&
			fa.AppliesToPackage( pkg ) &&
			fa.AppliesToPath( path ))
		{
			return true;
		}
	}
	return false;
}
bool HasWriteAccess( const Package* pkg, const Package* filepackage, const string& path )
{
	for( unsigned i = 0; i < file_access_rules.size(); ++i )
	{
		const FileAccess& fa = file_access_rules[i];
		if (fa.AllowWrite &&
			fa.AllowsAccessTo( pkg, filepackage ) &&
			fa.AppliesToPackage( pkg ) &&
			fa.AppliesToPath( path ))
		{
			return true;
		}
	}
	return false;
}
bool HasAppendAccess( const Package* pkg, const Package* filepackage, const string& path )
{
	for( unsigned i = 0; i < file_access_rules.size(); ++i )
	{
		const FileAccess& fa = file_access_rules[i];
		if (fa.AllowAppend &&
			fa.AllowsAccessTo( pkg, filepackage ) &&
			fa.AppliesToPackage( pkg ) &&
			fa.AppliesToPath( path ))
		{
			return true;
		}
	}
	return false;
}

ExecutorModule* CreateFileAccessExecutorModule( Executor& exec )
{
	return new FileAccessExecutorModule( exec );
}

BObjectImp* FileAccessExecutorModule::mf_FileExists()
{
	const String* filename;
	if ( !getStringParam(0, filename) )
		return new BError("Invalid parameter type.");

	const Package* outpkg;
	string path;
	if ( !pkgdef_split(filename->value(), exec.prog()->pkg, &outpkg, &path) )
		return new BError("Error in filename descriptor.");

	if ( path.find("..") != string::npos )
		return new BError("No parent path traversal allowed.");
	
	string filepath;
	if ( outpkg == NULL )
		filepath = path;
	else
		filepath = outpkg->dir() + path;

	return new BLong(FileExists(filepath));
}

BObjectImp* FileAccessExecutorModule::mf_ReadFile()
{
	const String* filename;
	if (!getStringParam( 0, filename ))
		return new BError( "Invalid parameter type" );

	const Package* outpkg;
	string path;
	if (!pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ))
		return new BError( "Error in filename descriptor" );

	if (path.find( ".." ) != string::npos)
		return new BError( "No parent path traversal please." );

	if (!HasReadAccess( exec.prog()->pkg, outpkg, path ))
		return new BError( "Access denied" );

	string filepath;
	if (outpkg == NULL)
		filepath = path;
	else
		filepath = outpkg->dir() + path;

	ifstream ifs( filepath.c_str() );
	if (!ifs.is_open())
		return new BError( "File not found: " + filepath );

	auto_ptr<ObjArray> arr (new ObjArray());

	string line;
	while (getline(ifs, line))
		arr->addElement( new String( line ) );

	return arr.release();
}

BObjectImp* FileAccessExecutorModule::mf_WriteFile()
{
	const String* filename;
	ObjArray* contents;
	if (!getStringParam( 0, filename ) ||
		!getObjArrayParam( 1, contents ))
	{
		return new BError( "Invalid parameter type" );
	}

	const Package* outpkg;
	string path;
	if (!pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ))
		return new BError( "Error in filename descriptor" );

	if (path.find( ".." ) != string::npos)
		return new BError( "No parent path traversal please." );

	if (!HasWriteAccess( exec.prog()->pkg, outpkg, path ))
		return new BError( "Access denied" );

	string filepath;
	if (outpkg == NULL)
		filepath = path;
	else
		filepath = outpkg->dir() + path;

	string bakpath = filepath + ".bak";
	string tmppath = filepath + ".tmp";

	ofstream ofs( tmppath.c_str(), ios::out|ios::trunc );

	if (!ofs.is_open())
		return new BError( "File not found: " + filepath );

	for( unsigned i = 0; i < contents->ref_arr.size(); ++i )
	{
		BObjectRef& ref = contents->ref_arr[i];
		BObject* obj = ref.get();
		if (obj != NULL)
		{
			ofs << (*obj)->getStringRep();
		}
		ofs << endl;
	}
	if (ofs.fail())
		return new BError( "Error during write." );
	ofs.close();

	if (FileExists( bakpath ))
	{
		if (unlink( bakpath.c_str() ))
		{
			int err = errno;
			string message = "Unable to remove " + filepath + ": " + strerror(err);

			return new BError( message );
		}
	}
	if (FileExists( filepath ))
	{
		if (rename( filepath.c_str(), bakpath.c_str() ))
		{
			int err = errno;
			string message = "Unable to rename " + filepath + " to " + bakpath + ": " + strerror(err);
			return new BError( message );
		}
	}
	if (rename( tmppath.c_str(), filepath.c_str() ))
	{
		int err = errno;
		string message = "Unable to rename " + tmppath + " to " + filepath + ": " + strerror(err);
		return new BError( message );
	}

	return new BLong(1);
}

BObjectImp* FileAccessExecutorModule::mf_AppendToFile()
{
	const String* filename;
	ObjArray* contents;
	if (!getStringParam( 0, filename ) ||
		!getObjArrayParam( 1, contents ))
	{
		return new BError( "Invalid parameter type" );
	}

	const Package* outpkg;
	string path;
	if (!pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ))
		return new BError( "Error in filename descriptor" );

	if (path.find( ".." ) != string::npos)
		return new BError( "No parent path traversal please." );

	if (!HasAppendAccess( exec.prog()->pkg, outpkg, path ))
		return new BError( "Access denied" );

	string filepath;
	if (outpkg == NULL)
		filepath = path;
	else
		filepath = outpkg->dir() + path;

	ofstream ofs( filepath.c_str(), ios::out|ios::app );

	if (!ofs.is_open())
		return new BError( "Unable to open file: " + filepath );

	for( unsigned i = 0; i < contents->ref_arr.size(); ++i )
	{
		BObjectRef& ref = contents->ref_arr[i];
		BObject* obj = ref.get();
		if (obj != NULL)
		{
			ofs << (*obj)->getStringRep();
		}
		ofs << endl;
	}
	if (ofs.fail())
		return new BError( "Error during write." );

	return new BLong(1);
}

BObjectImp* FileAccessExecutorModule::mf_LogToFile()
{ //
	const String* filename;
	const String* textline;
	if (getStringParam( 0, filename ) &&
		getStringParam( 1, textline ))
	{
        int flags;
		if (exec.fparams.size() >= 3)
		{
			if (!getParam( 2, flags ))
				return new BError( "Invalid parameter type" );
		}
		else
		{
			flags = 0;
		}

		const Package* outpkg;
		string path;
		if (!pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ))
			return new BError( "Error in filename descriptor" );
		
		if (path.find( ".." ) != string::npos)
			return new BError( "No parent path traversal please." );
		
		if (!HasAppendAccess( exec.prog()->pkg, outpkg, path ))
			return new BError( "Access denied" );
		
		string filepath;
		if (outpkg == NULL)
			filepath = path;
		else
			filepath = outpkg->dir() + path;
		
		ofstream ofs( filepath.c_str(), ios::out|ios::app );
		
		if (!ofs.is_open())
			return new BError( "Unable to open file: " + filepath );
		
		if (flags & LOG_DATETIME)
		{
			time_t now = time(NULL);
			struct tm* tm_now = localtime( &now );
			
			char buffer[30];
			strftime( buffer, sizeof buffer, "%m/%d %H:%M:%S", tm_now );
			ofs << "[" << buffer << "] ";
		}
		
		ofs << textline->value() << endl;
		
		if (ofs.fail())
			return new BError( "Error during write." );
		
		return new BLong(1);
	}
	else
		return new BError( "Invalid parameter type" );
}

BObjectImp* FileAccessExecutorModule::mf_OpenBinaryFile()
{
	const String* filename;
	unsigned short mode,bigendian;
	if ((!getStringParam( 0, filename )) ||
		(!getParam(1,mode)) ||
		(!getParam(2,bigendian)))
		return new BError( "Invalid parameter type" );

	const Package* outpkg;
	string path;
	if (!pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ))
		return new BError( "Error in filename descriptor" );

	if (path.find( ".." ) != string::npos)
		return new BError( "No parent path traversal please." );

	if (mode & 0x01)
	{
		if (!HasReadAccess( exec.prog()->pkg, outpkg, path ))
			return new BError( "Access denied" );
	}
	if (mode & 0x02)
	{
		if (!HasWriteAccess( exec.prog()->pkg, outpkg, path ))
			return new BError( "Access denied" );
	}

	string filepath;
	if (outpkg == NULL)
		filepath = path;
	else
		filepath = outpkg->dir() + path;

	return new BBinaryfile( filepath, mode, bigendian==1?true:false );

}

BObjectImp* FileAccessExecutorModule::mf_CreateDirectory()
{
	const String* dirname;
	if (!getStringParam( 0, dirname ))
		return new BError( "Invalid parameter type" );

	const Package* outpkg;
	string path;
	if (!pkgdef_split( dirname->value(), exec.prog()->pkg, &outpkg, &path ))
		return new BError( "Error in dirname descriptor" );
	if (path.find( ".." ) != string::npos)
		return new BError( "No parent path traversal please." );

	if (outpkg != NULL)
		path = outpkg->dir() + path;
	path = normalized_dir_form(path);
	if ( IsDirectory( path.c_str() ) )
		return new BError( "Directory already exists." );
	int res = make_dir(path.c_str());
	if (res != 0)
		return new BError( "Could not create directory." );
	return new BLong(1);
}

BObjectImp* FileAccessExecutorModule::mf_ListDirectory()
{
	const String* dirname;
	const String* extension;
	short listdirs;
	if ((!getStringParam( 0, dirname )) ||
		(!getStringParam( 1, extension )) ||
		(!getParam( 2, listdirs )) )
		return new BError( "Invalid parameter type" );

	const Package* outpkg;
	string path;
	if (!pkgdef_split( dirname->value(), exec.prog()->pkg, &outpkg, &path ))
		return new BError( "Error in dirname descriptor" );
	if (path.find( ".." ) != string::npos)
		return new BError( "No parent path traversal please." );

	if (outpkg != NULL)
		path = outpkg->dir() + path;
	path = normalized_dir_form(path);
	if ( !IsDirectory( path.c_str() ) )
		return new BError( "Directory not found." );
	bool asterisk = false;
	bool nofiles = false;
	if ( extension->getStringRep().find('*',0) != string::npos)
		asterisk = true;
	else if ( extension->length() == 0 )
		nofiles = true;

	ObjArray* arr = new ObjArray;

	for( DirList dl( path.c_str() ); !dl.at_end(); dl.next() )
	{
		string name = dl.name();
		if (name[0] == '.')
			continue;

		if ( IsDirectory( (path+name).c_str() ) )
		{
			if ( listdirs == 0 )
				continue;
		}
		else if ( nofiles )
			continue;
		else if (!asterisk)
		{
			string::size_type extensionPointPos = name.rfind('.');
			if (extensionPointPos == string::npos)
				continue;
			if (name.substr(extensionPointPos + 1) != extension->value())
				continue;
		}
		
		arr->addElement( new String(name) );
	}

	return arr;
}

BObjectImp* FileAccessExecutorModule::mf_OpenXMLFile()
{
	const String* filename;
	if (!getStringParam( 0, filename ))
		return new BError( "Invalid parameter type" );

	const Package* outpkg;
	string path;
	if (!pkgdef_split( filename->value(), exec.prog()->pkg, &outpkg, &path ))
		return new BError( "Error in filename descriptor" );

	if (path.find( ".." ) != string::npos)
		return new BError( "No parent path traversal please." );

	if (!HasReadAccess( exec.prog()->pkg, outpkg, path ))
		return new BError( "Access denied" );

	string filepath;
	if (outpkg == NULL)
		filepath = path;
	else
		filepath = outpkg->dir() + path;

	return new BXMLfile( filepath );
}

void load_fileaccess_cfg()
{
	file_access_rules.clear();

	if (!FileExists( "config/fileaccess.cfg" ))
		return;

	ConfigFile cf( "config/fileaccess.cfg", "FileAccess" );
	ConfigElem elem;


	while (cf.read( elem ))
	{
		FileAccess fa( elem );
		file_access_rules.push_back( fa );
	}
}

