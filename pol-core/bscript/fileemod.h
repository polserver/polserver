/*
History
=======

Notes
=======

*/

class FileExecutorModule;

typedef BObjectImp* (FileExecutorModule::*FileExecutorModuleFn)();

#ifdef _MSC_VER
#	pragma pack( push, 1 )
#else
/* Ok, my build of GCC supports this, yay! */
#	undef POSTSTRUCT_PACK
#	define POSTSTRUCT_PACK
#	pragma pack(1)
#endif
struct FileFunctionDef
{
	char funcname[ 33 ];
	FileExecutorModuleFn fptr;
};
#ifdef _MSC_VER
#	pragma pack( pop )
#else
#	pragma pack()
#endif
 

class FileExecutorModule : public ExecutorModule
{
  public:
	BObjectImp* FileOpen();
	BObjectImp* FileClose();
	BObjectImp* FileRead();
	BObjectImp* FileWrite();
	BObjectImp* FileSeek();
	BObjectImp* FileTell();

    explicit FileExecutorModule(Executor& exec) : ExecutorModule("File", exec) { }

	// class machinery
  protected:
	virtual BObjectImp* execFunc( unsigned idx );
	virtual int functionIndex( const char *func );
	static FileFunctionDef function_table[];
};
