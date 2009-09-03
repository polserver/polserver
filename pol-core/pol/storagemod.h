/*
History
=======
Make sure to set the module up in scrsched.cpp too
add_common_exmods()


Notes
=======

*/

#ifndef STORAGEEMOD_H
#define STORAGEEMOD_H

#include "bscript/execmodl.h"

class StorageExecutorModule : public TmplExecutorModule<StorageExecutorModule>
{
public:
	StorageExecutorModule( Executor& exec ) :
	TmplExecutorModule<StorageExecutorModule>("Storage", exec) {};

	BObjectImp* mf_StorageAreas();		
	BObjectImp* mf_DestroyRootItemInStorageArea();
	BObjectImp* mf_FindStorageArea();
	BObjectImp* mf_CreateStorageArea();
	BObjectImp* mf_FindRootItemInStorageArea();
	BObjectImp* mf_CreateRootItemInStorageArea();
};

#endif
