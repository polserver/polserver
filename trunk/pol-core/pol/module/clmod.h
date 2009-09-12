/*
History
=======

Notes
=======

*/

#ifndef CLILOCEMOD_H
#define CLILOCEMOD_H

#include "../../bscript/execmodl.h"

class ClilocExecutorModule : public TmplExecutorModule<ClilocExecutorModule>
{
public:
	ClilocExecutorModule( Executor& exec ) :
	TmplExecutorModule<ClilocExecutorModule>( "cliloc", exec ) {};

	BObjectImp* mf_SendSysMessageCL( );
	BObjectImp* mf_PrintTextAboveCL( );
	BObjectImp* mf_PrintTextAbovePrivateCL( );

};


#endif
