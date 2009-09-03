/*
History
=======
2005/09/10 Shinigami: added mf_Root - calculates y Root of x as a Real

Notes
=======

*/

#ifndef BSCRIPT_MATHEMOD_H
#define BSCRIPT_MATHEMOD_H

#include "bscript/execmodl.h"

class MathExecutorModule : public TmplExecutorModule<MathExecutorModule>
{
public:
    MathExecutorModule( Executor& exec ) :
        TmplExecutorModule<MathExecutorModule>( "math", exec ) {};

    BObjectImp* mf_Sin();
    BObjectImp* mf_ASin();
    BObjectImp* mf_Cos();
    BObjectImp* mf_ACos();
    BObjectImp* mf_Tan();
    BObjectImp* mf_ATan();

    BObjectImp* mf_Min();
    BObjectImp* mf_Max();
    BObjectImp* mf_Pow();
    BObjectImp* mf_Sqrt();
    BObjectImp* mf_Root();
    BObjectImp* mf_Abs();
    BObjectImp* mf_Log10();
    BObjectImp* mf_LogE();

    BObjectImp* mf_ConstPi();
    BObjectImp* mf_ConstE();

    BObjectImp* mf_FormatRealToString();

    BObjectImp* mf_RadToDeg();
    BObjectImp* mf_DegToRad();

    BObjectImp* mf_Ceil();
    BObjectImp* mf_Floor();
};

#endif
