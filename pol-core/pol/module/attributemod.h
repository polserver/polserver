/** @file
 *
 * @par History
 *
 * @note Make sure to set the module up in scrsched.cpp too
 * add_common_exmods()
 */


#ifndef ATTRIBUTEEMOD_H
#define ATTRIBUTEEMOD_H

#include "../polmodl.h"


namespace Pol::Bscript
{
class BObjectImp;
class Executor;
}  // namespace Pol::Bscript


namespace Pol::Module
{
class AttributeExecutorModule
    : public Bscript::TmplExecutorModule<AttributeExecutorModule, Core::PolModule>
{
public:
  AttributeExecutorModule( Bscript::Executor& exec );

  [[nodiscard]] Bscript::BObjectImp* mf_CheckSkill();  // Character, SkillId, Difficulty, Points

  [[nodiscard]] Bscript::BObjectImp* mf_GetAttributeName( /* alias name */ );

  [[nodiscard]] Bscript::BObjectImp* mf_GetAttributeDefaultCap( /* alias name */ );

  [[nodiscard]] Bscript::BObjectImp* mf_GetAttribute( /* mob, attrname */ );
  [[nodiscard]] Bscript::BObjectImp* mf_GetAttributeBaseValue( /* mob, attrname */ );
  [[nodiscard]] Bscript::BObjectImp* mf_GetAttributeTemporaryMod( /* mob, attrname */ );
  [[nodiscard]] Bscript::BObjectImp* mf_GetAttributeIntrinsicMod( /* mob, attrname */ );
  [[nodiscard]] Bscript::BObjectImp* mf_GetAttributeLock( /* mob, attrname */ );
  [[nodiscard]] Bscript::BObjectImp* mf_GetAttributeCap( /* mob, attrname */ );

  [[nodiscard]] Bscript::BObjectImp* mf_SetAttributeCap( /* mob, attrname, value */ );
  [[nodiscard]] Bscript::BObjectImp* mf_SetAttributeLock( /* mob, attrname, lockstate */ );
  [[nodiscard]] Bscript::BObjectImp* mf_SetAttributeBaseValue( /* mob, attributeid, basevalue */ );
  [[nodiscard]] Bscript::BObjectImp* mf_SetAttributeTemporaryMod(
      /* mob, attributeid, temporary_mod */ );
  [[nodiscard]] Bscript::BObjectImp* mf_AlterAttributeTemporaryMod(
      /* mob, attributeid, temporary_mod */ );

  [[nodiscard]] Bscript::BObjectImp* mf_RawSkillToBaseSkill();
  [[nodiscard]] Bscript::BObjectImp* mf_BaseSkillToRawSkill();
};
}  // namespace Pol::Module

#endif
