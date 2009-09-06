/*
History
=======


Notes
=======

*/

#ifndef DATASTORE_H
#define DATASTORE_H

#include "../../bscript/execmodl.h"

class DataStoreFile;

class DataFileExecutorModule : public TmplExecutorModule<DataFileExecutorModule>
{
public:
    DataFileExecutorModule( Executor& exec ) :
        TmplExecutorModule<DataFileExecutorModule>( "datafile", exec ) {}

	BObjectImp* mf_ListDataFiles();
	BObjectImp* mf_CreateDataFile();
	BObjectImp* mf_OpenDataFile();
	BObjectImp* mf_UnloadDataFile();

private:
    DataStoreFile* GetDataStoreFile( const std::string& inspec );
};

#endif
