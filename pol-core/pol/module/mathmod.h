/** @file
 *
 * @par History
 * - 2005/09/10 Shinigami: added mf_Root - calculates y Root of x as a Real
 */


#ifndef BSCRIPT_MATHEMOD_H
#define BSCRIPT_MATHEMOD_H

#include "../../bscript/execmodl.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Module
{
class MathExecutorModule
    : public Bscript::TmplExecutorModule<MathExecutorModule, Bscript::ExecutorModule>
{
public:
  MathExecutorModule( Bscript::Executor& exec );

  Bscript::BObjectImp* mf_Sin();
  Bscript::BObjectImp* mf_ASin();
  Bscript::BObjectImp* mf_Cos();
  Bscript::BObjectImp* mf_ACos();
  Bscript::BObjectImp* mf_Tan();
  Bscript::BObjectImp* mf_ATan();

  Bscript::BObjectImp* mf_Min();
  Bscript::BObjectImp* mf_Max();
  Bscript::BObjectImp* mf_Pow();
  Bscript::BObjectImp* mf_Sqrt();
  Bscript::BObjectImp* mf_Root();
  Bscript::BObjectImp* mf_Abs();
  Bscript::BObjectImp* mf_Log10();
  Bscript::BObjectImp* mf_LogE();

  Bscript::BObjectImp* mf_ConstPi();
  Bscript::BObjectImp* mf_ConstE();

  Bscript::BObjectImp* mf_FormatRealToString();

  Bscript::BObjectImp* mf_RadToDeg();
  Bscript::BObjectImp* mf_DegToRad();

  Bscript::BObjectImp* mf_Ceil();
  Bscript::BObjectImp* mf_Floor();
};
}  // namespace Module
}  // namespace Pol
#endif
