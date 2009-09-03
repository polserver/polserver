/*
History
=======
2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule

Notes
=======

*/

#include "clib/stl_inc.h"

#include "bscript/bobject.h"
#include "bscript/berror.h"
#include "bscript/impstr.h"

#include "attribute.h"
#include "attributemod.h"
#include "charactr.h"
#include "cliface.h"
#include "skilladv.h"
#include "uoexhelp.h"

template<>
TmplExecutorModule<AttributeExecutorModule>::FunctionDef
	TmplExecutorModule<AttributeExecutorModule>::function_table[] =
{
	{ "CheckSkill",						&AttributeExecutorModule::mf_CheckSkill },
	{ "GetAttributeName",				&AttributeExecutorModule::mf_GetAttributeName },
	{ "GetAttributeDefaultCap",			&AttributeExecutorModule::mf_GetAttributeDefaultCap },
	{ "GetAttribute",					&AttributeExecutorModule::mf_GetAttribute },
	{ "GetAttributeBaseValue",			&AttributeExecutorModule::mf_GetAttributeBaseValue },
	{ "GetAttributeTemporaryMod",		&AttributeExecutorModule::mf_GetAttributeTemporaryMod },
	{ "GetAttributeIntrinsicMod",		&AttributeExecutorModule::mf_GetAttributeIntrinsicMod },
	{ "GetAttributeLock",				&AttributeExecutorModule::mf_GetAttributeLock },
	{ "GetAttributeCap",				&AttributeExecutorModule::mf_GetAttributeCap },
	{ "SetAttributeCap",				&AttributeExecutorModule::mf_SetAttributeCap },
	{ "SetAttributeLock",				&AttributeExecutorModule::mf_SetAttributeLock },
	{ "SetAttributeBaseValue",			&AttributeExecutorModule::mf_SetAttributeBaseValue },
	{ "SetAttributeTemporaryMod",		&AttributeExecutorModule::mf_SetAttributeTemporaryMod },
	{ "AlterAttributeTemporaryMod",		&AttributeExecutorModule::mf_AlterAttributeTemporaryMod },
	{ "RawSkillToBaseSkill",			&AttributeExecutorModule::mf_RawSkillToBase },
	{ "BaseSkillToRawSkill",			&AttributeExecutorModule::mf_BaseSkillToRaw }
};
template<>
int TmplExecutorModule<AttributeExecutorModule>::function_table_size =
	arsize(function_table);


BObjectImp* AttributeExecutorModule::mf_CheckSkill()
{
	Character* chr;
	USKILLID skillid;
	long difficulty;
	unsigned short points;

	if ( getCharacterParam(exec, 0, chr) &&
		getSkillIdParam(exec, 1, skillid) &&
		getParam(2, difficulty) &&
		getParam(3, points) )
	{
		return new BLong( chr->check_skill( skillid, difficulty, points ) );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

BObjectImp* AttributeExecutorModule::mf_GetAttributeName( /* alias_name */ )
{
	const Attribute* attr;

	if ( !getAttributeParam(exec, 0, attr) )
	{
		return new BError("Invalid parameter type.");
	}

	return new String(attr->name);
}

BObjectImp* AttributeExecutorModule::mf_GetAttributeDefaultCap ( /* alias_name */ )
{
	const Attribute* attr;

	if ( !getAttributeParam(exec, 0, attr) )
	{
		return new BError("Invalid parameter type.");
	}

	return new BLong(attr->default_cap);
}

BObjectImp* AttributeExecutorModule::mf_GetAttribute( /* mob, attrname */ )
{
	Character* chr;
	const Attribute* attr;

	if ( getCharacterParam(exec, 0, chr) &&
		getAttributeParam(exec, 1, attr) )
	{
		const AttributeValue& av = chr->attribute(attr->attrid);
		return new BLong(av.effective());
	}
	else
	{
		return new BError("Invalid parameter type");
	}
}

BObjectImp* AttributeExecutorModule::mf_GetAttributeBaseValue( /* mob, attrname */ )
{
	Character* chr;
	const Attribute* attr;

	if ( getCharacterParam(exec, 0, chr) &&
		getAttributeParam(exec, 1, attr) )
	{
		const AttributeValue& av = chr->attribute(attr->attrid);
		return new BLong(av.base());
	}
	else
	{
		return new BError("Invalid parameter type");
	}
}

BObjectImp* AttributeExecutorModule::mf_GetAttributeTemporaryMod( /* mob, attrname */ )
{
	Character* chr;
	const Attribute* attr;

	if ( getCharacterParam(exec, 0, chr) &&
		getAttributeParam(exec, 1, attr) )
	{
		const AttributeValue& av = chr->attribute(attr->attrid);
		return new BLong(av.temp_mod());
	}
	else
	{
		return new BError("Invalid parameter type");
	}
}

BObjectImp* AttributeExecutorModule::mf_GetAttributeIntrinsicMod( /* mob, attrname */ )
{
	Character* chr;
	const Attribute* attr;

	if ( getCharacterParam(exec, 0, chr) &&
		getAttributeParam(exec, 1, attr) )
	{
		const AttributeValue& av = chr->attribute(attr->attrid);
		return new BLong(av.intrinsic_mod());
	}
	else
	{
		return new BError("Invalid parameter type");
	}
}

BObjectImp* AttributeExecutorModule::mf_GetAttributeLock( /* mob, attrname */ )
{
	Character* chr;
	const Attribute* attr;

	if ( getCharacterParam(exec, 0, chr) &&
		getAttributeParam(exec, 1, attr) )
	{
		const AttributeValue& av = chr->attribute(attr->attrid);
		return new BLong(av.lock());
	}
	else
	{
		return new BError("Invalid parameter type");
	}
}
BObjectImp* AttributeExecutorModule::mf_GetAttributeCap( /* mob, attrname */ )
{
	Character* chr;
	const Attribute* attr;

	if ( getCharacterParam(exec, 0, chr) &&
		getAttributeParam(exec, 1, attr) )
	{
		const AttributeValue& av = chr->attribute(attr->attrid);
		return new BLong(av.cap());
	}
	else
	{
		return new BError("Invalid parameter type");
	}
}

BObjectImp* AttributeExecutorModule::mf_SetAttributeCap( /* mob, attributeid, capvalue */ )
{
	Character* chr;
	const Attribute* attr;
	unsigned short capvalue;

	// FIXME: SetAttributeCap(mob, attributeid) should reset cap to default value :)

	if ( getCharacterParam(exec, 0, chr) &&
		getAttributeParam(exec, 1, attr) &&
		getParam(2, capvalue, ATTRIBUTE_MIN_BASE, ATTRIBUTE_MAX_BASE ) )
	{
		chr->set_dirty();
		AttributeValue& av = chr->attribute(attr->attrid);
		long old_cap = av.cap();
		av.cap(capvalue);

		if ( old_cap != capvalue )
		{
			ClientInterface::tell_attribute_changed(chr,attr);
		}

		return new BLong( 1 );
	}
	else
	{
		return new BError("Invalid parameter type");
	}
}

BObjectImp* AttributeExecutorModule::mf_SetAttributeLock( /* mob, attributeid, lockstate */ )
{
	Character* chr;
	const Attribute* attr;
	unsigned short lockstate;

	if ( getCharacterParam(exec, 0, chr) &&
		getAttributeParam(exec, 1, attr) &&
		getParam(2, lockstate, 0, 2 ) ) // FIXME: hard-coded lock states min and max (0 and 2) -- Nando
	{
		chr->set_dirty();
		AttributeValue& av = chr->attribute(attr->attrid);

		unsigned char old_state = av.lock();
		av.lock((unsigned char)lockstate);

		if ( old_state != lockstate )
		{
			ClientInterface::tell_attribute_changed(chr,attr);
		}

		return new BLong( 1 );
	}
	else
	{
		return new BError("Invalid parameter type");
	}
}

BObjectImp* AttributeExecutorModule::mf_SetAttributeBaseValue( /* mob, attributeid, basevalue */ )
{
	Character* chr;
	const Attribute* attr;
	unsigned short basevalue;

	if ( getCharacterParam(exec, 0, chr) &&
		getAttributeParam(exec, 1, attr) &&
		getParam(2, basevalue, ATTRIBUTE_MIN_BASE, ATTRIBUTE_MAX_BASE ) )
	{
		chr->set_dirty();
		AttributeValue& av = chr->attribute(attr->attrid);
		long eff = av.effective_tenths();
		av.base(basevalue);

		if ( eff != av.effective_tenths() )
		{
			ClientInterface::tell_attribute_changed(chr,attr);
			if ( attr->attrid == pAttrParry->attrid )
			{
				if (chr->has_shield())
					chr->refresh_ar();
			}
		}

		return new BLong( 1 );
	}
	else
	{
		return new BError("Invalid parameter type");
	}
}

BObjectImp* AttributeExecutorModule::mf_SetAttributeTemporaryMod( /* mob, attributeid, temporary_mod */ )
{
	Character* chr;
	const Attribute* attr;
	long tempmod;

	if ( getCharacterParam(exec, 0, chr) &&
		getAttributeParam(exec, 1, attr) &&
		getParam(2, tempmod, ATTRIBUTE_MIN_TEMP_MOD, ATTRIBUTE_MAX_TEMP_MOD) )
	{
		AttributeValue& av = chr->attribute(attr->attrid);
		long eff = av.effective_tenths();
		av.temp_mod(static_cast<short>(tempmod));

		if ( eff != av.effective_tenths() )
		{
			ClientInterface::tell_attribute_changed(chr,attr);
			if ( attr->attrid == pAttrParry->attrid )
			{
				if (chr->has_shield())
					chr->refresh_ar();
			}
		}
		
		return new BLong(1);
	}
	else
	{
		return new BError("Invalid parameter type");
	}
}

BObjectImp* AttributeExecutorModule::mf_AlterAttributeTemporaryMod( /* mob, attributeid, temporary_mod */ )
{
	Character* chr;
	const Attribute* attr;
	long delta;

	if ( getCharacterParam(exec, 0, chr) &&
		getAttributeParam(exec, 1, attr) &&
		getParam(2, delta ) )
	{
		AttributeValue& av = chr->attribute(attr->attrid);
		long eff = av.effective_tenths();
		long newmod = av.temp_mod() + delta;

		if ( newmod < ATTRIBUTE_MIN_TEMP_MOD || newmod > ATTRIBUTE_MAX_TEMP_MOD )
			return new BError( "New modifier value is out of range" );
		
		av.temp_mod(static_cast<short>(newmod));
		
		if (eff != av.effective_tenths())
		{
			ClientInterface::tell_attribute_changed(chr,attr);
			if ( attr->attrid == pAttrParry->attrid )
			{
				if (chr->has_shield())
					chr->refresh_ar();
			}
		}
		
		return new BLong(1);
	}
	else
	{
		return new BError("Invalid parameter type");
	}
}


BObjectImp* AttributeExecutorModule::mf_RawSkillToBase()
{
	long rawskill;
	if ( getParam(0, rawskill) )
	{
		if ( rawskill < 0 )
			rawskill = 0;
		return new BLong(raw_to_base(rawskill));
	}
	else
	{
		return new BError("Invalid parameter type");
	}
}

BObjectImp* AttributeExecutorModule::mf_BaseSkillToRaw()
{
	unsigned short baseskill;
	if ( getParam(0, baseskill) )
		return new BLong(base_to_raw(baseskill));
	else
		return new BError("Invalid parameter type");
}
