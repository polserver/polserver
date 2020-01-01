/** @file
 *
 * @par History
 *
 * @note Make sure to set the module up in scrsched.cpp too
 * add_common_exmods()
 */


#ifndef ATTRIBUTEEMOD_H
#define ATTRIBUTEEMOD_H

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
class AttributeExecutorModule : public Bscript::TmplExecutorModule<AttributeExecutorModule>
{
public:
  AttributeExecutorModule( Bscript::Executor& exec );

  Bscript::BObjectImp* mf_CheckSkill();  // Character, SkillId, Difficulty, Points

  Bscript::BObjectImp* mf_GetAttributeName( /* alias name */ );

  Bscript::BObjectImp* mf_GetAttributeDefaultCap( /* alias name */ );

  Bscript::BObjectImp* mf_GetAttribute( /* mob, attrname */ );
  Bscript::BObjectImp* mf_GetAttributeBaseValue( /* mob, attrname */ );
  Bscript::BObjectImp* mf_GetAttributeTemporaryMod( /* mob, attrname */ );
  Bscript::BObjectImp* mf_GetAttributeIntrinsicMod( /* mob, attrname */ );
  Bscript::BObjectImp* mf_GetAttributeLock( /* mob, attrname */ );
  Bscript::BObjectImp* mf_GetAttributeCap( /* mob, attrname */ );

  Bscript::BObjectImp* mf_SetAttributeCap( /* mob, attrname, value */ );
  Bscript::BObjectImp* mf_SetAttributeLock( /* mob, attrname, lockstate */ );
  Bscript::BObjectImp* mf_SetAttributeBaseValue( /* mob, attributeid, basevalue */ );
  Bscript::BObjectImp* mf_SetAttributeTemporaryMod( /* mob, attributeid, temporary_mod */ );
  Bscript::BObjectImp* mf_AlterAttributeTemporaryMod( /* mob, attributeid, temporary_mod */ );
  Bscript::BObjectImp* mf_SetAttributeIntrinsicMod( /* mob, attributeid, intrinsic_mod */ );

  Bscript::BObjectImp* mf_RawSkillToBaseSkill();
  Bscript::BObjectImp* mf_BaseSkillToRawSkill();
};
}
}
#endif
