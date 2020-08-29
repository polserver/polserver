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

  [[nodiscard]] Bscript::BObjectImp* mf_Sin();
  [[nodiscard]] Bscript::BObjectImp* mf_ASin();
  [[nodiscard]] Bscript::BObjectImp* mf_Cos();
  [[nodiscard]] Bscript::BObjectImp* mf_ACos();
  [[nodiscard]] Bscript::BObjectImp* mf_Tan();
  [[nodiscard]] Bscript::BObjectImp* mf_ATan();

  [[nodiscard]] Bscript::BObjectImp* mf_Min();
  [[nodiscard]] Bscript::BObjectImp* mf_Max();
  [[nodiscard]] Bscript::BObjectImp* mf_Pow();
  [[nodiscard]] Bscript::BObjectImp* mf_Sqrt();
  [[nodiscard]] Bscript::BObjectImp* mf_Root();
  [[nodiscard]] Bscript::BObjectImp* mf_Abs();
  [[nodiscard]] Bscript::BObjectImp* mf_Log10();
  [[nodiscard]] Bscript::BObjectImp* mf_LogE();

  [[nodiscard]] Bscript::BObjectImp* mf_ConstPi();
  [[nodiscard]] Bscript::BObjectImp* mf_ConstE();

  [[nodiscard]] Bscript::BObjectImp* mf_FormatRealToString();

  [[nodiscard]] Bscript::BObjectImp* mf_RadToDeg();
  [[nodiscard]] Bscript::BObjectImp* mf_DegToRad();

  [[nodiscard]] Bscript::BObjectImp* mf_Ceil();
  [[nodiscard]] Bscript::BObjectImp* mf_Floor();
};
}  // namespace Module
}  // namespace Pol
#endif
