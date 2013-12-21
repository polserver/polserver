/*
History
=======
2009/12/18 Turley:    added CreateDirectory() & ListDirectory()


Notes
=======

*/

#ifndef FILEACCESS_H
#define FILEACCESS_H
namespace Pol {
  namespace Bscript {
	class ExecutorModule;
	class Executor;
  }
  namespace Module {
	class FileAccessExecutorModule : public Bscript::TmplExecutorModule<FileAccessExecutorModule>
	{
	public:
	  FileAccessExecutorModule( Bscript::Executor& exec ) :
		TmplExecutorModule<FileAccessExecutorModule>( "file", exec ) {}

	  Bscript::BObjectImp* mf_FileExists();
	  Bscript::BObjectImp* mf_ReadFile();
	  Bscript::BObjectImp* mf_WriteFile();
	  Bscript::BObjectImp* mf_AppendToFile();
	  Bscript::BObjectImp* mf_LogToFile();
	  Bscript::BObjectImp* mf_OpenBinaryFile();
	  Bscript::BObjectImp* mf_CreateDirectory();
	  Bscript::BObjectImp* mf_ListDirectory();
	  Bscript::BObjectImp* mf_OpenXMLFile();
	  Bscript::BObjectImp* mf_CreateXMLFile();
	};

	Bscript::ExecutorModule* CreateFileAccessExecutorModule( Bscript::Executor& exec );

	class FileAccess
	{
	public:
	  explicit FileAccess( Clib::ConfigElem& elem );
	  bool AllowsAccessTo( const Plib::Package* pkg, const Plib::Package* filepackage ) const;
	  bool AppliesToPackage( const Plib::Package* pkg ) const;
	  bool AppliesToPath( const std::string& path ) const;

	  bool AllowWrite;
	  bool AllowAppend;
	  bool AllowRead;

	  bool AllowRemote;

	  bool AllPackages;
	  bool AllDirectories; // not used
	  bool AllExtensions;

	  set<std::string, Clib::ci_cmp_pred> Packages;
	  vector< std::string > Directories; // not used
	  vector< std::string > Extensions;
	};

	bool HasWriteAccess( const Plib::Package* pkg, const Plib::Package* filepackage, const std::string& path );
  }
}
#endif
