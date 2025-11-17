/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_BASICMOD_H
#define BSCRIPT_BASICMOD_H

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
}  // namespace Bscript
}  // namespace Pol
#ifndef BSCRIPT_EXECMODL_H
#include "../../bscript/execmodl.h"
#endif

// FIXME: this is currently misnamed, should be StringExecutorModule
namespace Pol
{
namespace Module
{
class BasicExecutorModule
    : public Bscript::TmplExecutorModule<BasicExecutorModule, Bscript::ExecutorModule>
{
public:
  /* These probably belong in a string module */
  [[nodiscard]] Bscript::BObjectImp* mf_Find();
  [[nodiscard]] Bscript::BObjectImp* mf_Len();
  [[nodiscard]] Bscript::BObjectImp* mf_Upper();
  [[nodiscard]] Bscript::BObjectImp* mf_Lower();
  [[nodiscard]] Bscript::BObjectImp* mf_SubStr();
  [[nodiscard]] Bscript::BObjectImp* mf_Trim();
  [[nodiscard]] Bscript::BObjectImp* mf_StrReplace();
  [[nodiscard]] Bscript::BObjectImp* mf_SubStrReplace();
  [[nodiscard]] Bscript::BObjectImp* mf_Compare();
  [[nodiscard]] Bscript::BObjectImp* mf_CInt();
  [[nodiscard]] Bscript::BObjectImp* mf_CStr();
  [[nodiscard]] Bscript::BObjectImp* mf_CDbl();
  [[nodiscard]] Bscript::BObjectImp* mf_CAsc();
  [[nodiscard]] Bscript::BObjectImp* mf_CAscZ();
  [[nodiscard]] Bscript::BObjectImp* mf_CChr();
  [[nodiscard]] Bscript::BObjectImp* mf_CChrZ();

  [[nodiscard]] Bscript::BObjectImp* mf_Bin();
  [[nodiscard]] Bscript::BObjectImp* mf_Hex();
  [[nodiscard]] Bscript::BObjectImp* mf_SplitWords();

  [[nodiscard]] Bscript::BObjectImp* mf_Pack();
  [[nodiscard]] Bscript::BObjectImp* mf_Unpack();

  [[nodiscard]] Bscript::BObjectImp* mf_TypeOf();
  [[nodiscard]] Bscript::BObjectImp* mf_SizeOf();
  [[nodiscard]] Bscript::BObjectImp* mf_TypeOfInt();

  [[nodiscard]] Bscript::BObjectImp* mf_PackJSON();
  [[nodiscard]] Bscript::BObjectImp* mf_UnpackJSON();

  [[nodiscard]] Bscript::BObjectImp* mf_Boolean();

  [[nodiscard]] Bscript::BObjectImp* mf_EncodeBase64();
  [[nodiscard]] Bscript::BObjectImp* mf_DecodeBase64();
  [[nodiscard]] Bscript::BObjectImp* mf_RegExp();

  BasicExecutorModule( Bscript::Executor& exec );
};
}  // namespace Module
}  // namespace Pol
#endif
