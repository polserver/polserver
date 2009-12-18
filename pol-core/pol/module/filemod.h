/*
History
=======
2009/12/18 Turley:    added CreateDirectory() & ListDirectory()


Notes
=======

*/

#ifndef FILEACCESS_H
#define FILEACCESS_H

class ExecutorModule;
class Executor;

class FileAccessExecutorModule : public TmplExecutorModule<FileAccessExecutorModule>
{
public:
	FileAccessExecutorModule( Executor& exec ) :
	  TmplExecutorModule<FileAccessExecutorModule>( "file", exec ) {}

	  BObjectImp* mf_FileExists();
	  BObjectImp* mf_ReadFile();
	  BObjectImp* mf_WriteFile();
	  BObjectImp* mf_AppendToFile();
	  BObjectImp* mf_LogToFile();
	  BObjectImp* mf_OpenBinaryFile();
	  BObjectImp* mf_CreateDirectory();
	  BObjectImp* mf_ListDirectory();
};

ExecutorModule* CreateFileAccessExecutorModule( Executor& exec );

class FileAccess
{
public:
	explicit FileAccess( ConfigElem& elem );
	bool AllowsAccessTo( const Package* pkg, const Package* filepackage ) const;
	bool AppliesToPackage( const Package* pkg ) const;
	bool AppliesToPath( const string& path ) const;

	bool AllowWrite;
	bool AllowAppend;
	bool AllowRead;

	bool AllowRemote;

	bool AllPackages;
	bool AllDirectories; // not used
	bool AllExtensions;

	set<string, ci_cmp_pred> Packages;
	vector< string > Directories; // not used
	vector< string > Extensions;
};

#endif
