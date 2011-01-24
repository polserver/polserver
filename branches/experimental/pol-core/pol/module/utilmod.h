/*
History
=======
2008/07/08 Turley: Added mf_RandomIntMinMax - Return Random Value between...

Notes
=======

*/

#ifndef __UTILMOD_H
#define __UTILMOD_H

class UtilExecutorModule : public TmplExecutorModule<UtilExecutorModule>
{
public:
    UtilExecutorModule( Executor& exec ) : TmplExecutorModule<UtilExecutorModule>("util",exec) {}

    BObjectImp* mf_RandomInt();
    BObjectImp* mf_RandomFloat();
    BObjectImp* mf_RandomDiceRoll();
	BObjectImp* mf_StrFormatTime();
	BObjectImp* mf_RandomIntMinMax();
};

#endif // UTILMOD_H

