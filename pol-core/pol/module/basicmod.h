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
class BasicExecutorModule : public Bscript::TmplExecutorModule<BasicExecutorModule>
{
public:
  /* These probably belong in a string module */
  Bscript::BObjectImp* mf_Find();
  Bscript::BObjectImp* mf_Len();
  Bscript::BObjectImp* mf_Upper();
  Bscript::BObjectImp* mf_Lower();
  Bscript::BObjectImp* mf_SubStr();
  Bscript::BObjectImp* mf_Trim();
  Bscript::BObjectImp* mf_StrReplace();
  Bscript::BObjectImp* mf_SubStrReplace();
  Bscript::BObjectImp* mf_Compare();
  Bscript::BObjectImp* mf_CInt();
  Bscript::BObjectImp* mf_CStr();
  Bscript::BObjectImp* mf_CDbl();
  Bscript::BObjectImp* mf_CAsc();
  Bscript::BObjectImp* mf_CAscZ();
  Bscript::BObjectImp* mf_CChr();
  Bscript::BObjectImp* mf_CChrZ();

  Bscript::BObjectImp* mf_Bin();
  Bscript::BObjectImp* mf_Hex();
  Bscript::BObjectImp* mf_SplitWords();

  Bscript::BObjectImp* mf_Pack();
  Bscript::BObjectImp* mf_Unpack();

  Bscript::BObjectImp* mf_TypeOf();
  Bscript::BObjectImp* mf_SizeOf();
  Bscript::BObjectImp* mf_TypeOfInt();

  Bscript::BObjectImp* mf_PackJSON();
  Bscript::BObjectImp* mf_UnpackJSON();

  Bscript::BObjectImp* mf_Boolean();

  Bscript::BObjectImp* mf_EncodeBase64();
  Bscript::BObjectImp* mf_DecodeBase64();

  BasicExecutorModule( Bscript::Executor& exec );
};
}  // namespace Module
}  // namespace Pol
#endif
