/*
History
=======
2008/07/08 Turley: Added mf_RandomIntMinMax - Return Random Value between...

Notes
=======

*/

#ifndef __UTILMOD_H
#define __UTILMOD_H

#include "../../bscript/execmodl.h"

namespace Pol {
  namespace Module {
	class UtilExecutorModule : public Bscript::TmplExecutorModule<UtilExecutorModule>
	{
	public:
      UtilExecutorModule( Bscript::Executor& exec ) : Bscript::TmplExecutorModule<UtilExecutorModule>( "util", exec ) {}

	  Bscript::BObjectImp* mf_RandomInt();
	  Bscript::BObjectImp* mf_RandomFloat();
	  Bscript::BObjectImp* mf_RandomDiceRoll();
	  Bscript::BObjectImp* mf_StrFormatTime();
	  Bscript::BObjectImp* mf_RandomIntMinMax();
	};
  }
}
#endif // UTILMOD_H

