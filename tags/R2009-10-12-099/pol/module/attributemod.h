/*
History
=======


Notes
=======
Make sure to set the module up in scrsched.cpp too
add_common_exmods()

*/

#ifndef ATTRIBUTEEMOD_H
#define ATTRIBUTEEMOD_H

#include "../../bscript/execmodl.h"

class AttributeExecutorModule : public TmplExecutorModule<AttributeExecutorModule>
{
public:
	AttributeExecutorModule( Executor& exec ) :
	TmplExecutorModule<AttributeExecutorModule>( "attributes", exec ) {};

	BObjectImp* mf_CheckSkill(); // Character, SkillId, Difficulty, Points

	BObjectImp* mf_GetAttributeName(/* alias name */);

	BObjectImp* mf_GetAttributeDefaultCap (/* alias name */);

	BObjectImp* mf_GetAttribute( /* mob, attrname */ );
	BObjectImp* mf_GetAttributeBaseValue( /* mob, attrname */ );
	BObjectImp* mf_GetAttributeTemporaryMod( /* mob, attrname */ );
	BObjectImp* mf_GetAttributeIntrinsicMod( /* mob, attrname */ );
	BObjectImp* mf_GetAttributeLock( /* mob, attrname */ );
	BObjectImp* mf_GetAttributeCap( /* mob, attrname */ );

	BObjectImp* mf_SetAttributeCap( /* mob, attrname, value */);
	BObjectImp* mf_SetAttributeLock( /* mob, attrname, lockstate */ );
	BObjectImp* mf_SetAttributeBaseValue( /* mob, attributeid, basevalue */ );
	BObjectImp* mf_SetAttributeTemporaryMod( /* mob, attributeid, temporary_mod */ );
	BObjectImp* mf_AlterAttributeTemporaryMod( /* mob, attributeid, temporary_mod */ );
	BObjectImp* mf_SetAttributeIntrinsicMod( /* mob, attributeid, intrinsic_mod */ );

	BObjectImp* mf_RawSkillToBase();
	BObjectImp* mf_BaseSkillToRaw();
};


#endif
