/*
History
=======
2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
2009/09/22 Turley:    repsys param to applydamage
2009/11/20 Turley:    RecalcVitals can update single Attributes/Vitals - based on Tomi

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include "../../bscript/bobject.h"
#include "../../bscript/berror.h"
#include "../../bscript/impstr.h"

#include "../mobile/charactr.h"
#include "../mobile/attribute.h"
#include "../network/cliface.h"
#include "../spells.h"
#include "../statmsg.h"
#include "../ufunc.h"
#include "uomod.h"
#include "../uoexhelp.h"
#include "../vital.h"
#include "vitalmod.h"

template<>
TmplExecutorModule<VitalExecutorModule>::FunctionDef
	TmplExecutorModule<VitalExecutorModule>::function_table[] =
{
	{ "ApplyRawDamage",				&VitalExecutorModule::mf_ApplyRawDamage },
	{ "ApplyDamage",				&VitalExecutorModule::mf_ApplyDamage },
	{ "HealDamage",					&VitalExecutorModule::mf_HealDamage },
	{ "ConsumeMana",				&VitalExecutorModule::mf_ConsumeMana },
	{ "ConsumeVital",				&VitalExecutorModule::mf_ConsumeVital },
	{ "RecalcVitals",				&VitalExecutorModule::mf_RecalcVitals },
	{ "GetVitalName",				&VitalExecutorModule::mf_GetVitalName },
	{ "GetVital",					&VitalExecutorModule::mf_GetVital },
	{ "SetVital",					&VitalExecutorModule::mf_SetVital },
	{ "GetVitalRegenRate",			&VitalExecutorModule::mf_GetVitalRegenRate },
	{ "GetVitalMaximumValue",		&VitalExecutorModule::mf_GetVitalMaximumValue }
};
template<>
int TmplExecutorModule<VitalExecutorModule>::function_table_size =
	arsize(function_table);

BObjectImp* VitalExecutorModule::mf_ApplyRawDamage()
{
	Character* chr;
	long damage;
    long userepsys;
	if ( getCharacterParam(exec, 0, chr) &&
		getParam(1, damage) &&
        getParam(2, userepsys) &&
		damage >= 0 &&
		damage <= USHRT_MAX )
	{
		chr->apply_raw_damage_hundredths(static_cast<unsigned long>(damage*100),
            GetUOController(), userepsys>0?true:false);
		return new BLong(1);
	}
	else
		return new BLong(0);
}

BObjectImp* VitalExecutorModule::mf_ApplyDamage()
{
	Character* chr;
	double damage;
    long userepsys;
	if ( getCharacterParam(exec, 0, chr) &&
		getRealParam( 1, damage) &&
        getParam( 2, userepsys))
	{
		if ( damage >= 0.0 && damage <= 30000.0 )
    {
      damage=chr->apply_damage(static_cast<unsigned short>(damage), GetUOController(),userepsys>0?true:false);
      return new BLong(static_cast<long>(damage));
		}
		else
			return new BError( "Damage is out of range" );
	}
	else
		return new BError( "Invalid parameter type" );
}

BObjectImp* VitalExecutorModule::mf_HealDamage()
{
	Character* chr;
	long amount;
	if ( getCharacterParam(exec, 0, chr) &&
		getParam(1, amount) &&
		amount >= 0 && amount <= USHRT_MAX )
	{
		Character* controller = GetUOController();
		if ( controller )
			controller->repsys_on_help(chr);

		chr->heal_damage_hundredths(static_cast<unsigned short>(amount) * 100L);
		return new BLong(1);
	}
	else
	{
		return new BError("Invalid parameter");
	}
}

BObjectImp* VitalExecutorModule::mf_ConsumeMana()
{
	Character* chr;
	long spellid;
	if ( getCharacterParam(exec, 0, chr) &&
		getParam(1, spellid) )
	{
		if ( !VALID_SPELL_ID(spellid) )
			return new BError( "Spell ID out of range" );

		USpell* spell = spells2[spellid];
		if ( spell == NULL )
			return new BError("No such spell");
		else if ( spell->check_mana(chr) == false )
			return new BLong(0);
		
		spell->consume_mana(chr);
		if ( chr->has_active_client() )
			send_mana_level(chr->client);

		return new BLong(1);
	}
	else
	{
		return new BError("Invalid parameter");
	}
}

BObjectImp* VitalExecutorModule::mf_GetVitalName( /* alias_name */ )
{
	const Vital* vital;

	if ( !getVitalParam(exec, 0, vital) )
	{
		return new BError("Invalid parameter type.");
	}

	return new String(vital->name);
}

BObjectImp* VitalExecutorModule::mf_GetVital( /* mob, vitalid */ )
{
	Character* chr;
	const Vital* vital;

	if ( getCharacterParam(exec, 0, chr) &&
		getVitalParam(exec, 1, vital) )
	{
		const VitalValue& vv = chr->vital(vital->vitalid);
		return new BLong(vv.current());
	}
	else
		return new BError("Invalid parameter type");
}

BObjectImp* VitalExecutorModule::mf_GetVitalMaximumValue( /* mob, vitalid */ )
{
	Character* chr;
	const Vital* vital;

	if ( getCharacterParam(exec, 0, chr) &&
		getVitalParam(exec, 1, vital) )
	{
		const VitalValue& vv = chr->vital(vital->vitalid);
		return new BLong(vv.maximum());
	}
	else
		return new BError("Invalid parameter type");
}

BObjectImp* VitalExecutorModule::mf_GetVitalRegenRate( /* mob, vitalid */ )
{
	Character* chr;
	const Vital* vital;

	if ( getCharacterParam(exec, 0, chr) &&
		getVitalParam(exec, 1, vital) )
	{
		const VitalValue& vv = chr->vital(vital->vitalid);
		return new BLong(vv.regenrate());
	}
	else
		return new BError("Invalid parameter type");
}

BObjectImp* VitalExecutorModule::mf_SetVital( /* mob, vitalid, hundredths */ )
{
	Character* chr;
	const Vital* vital;
	long value;

	if ( getCharacterParam(exec, 0, chr) &&
		getVitalParam(exec, 1, vital) &&
		getParam(2, value, VITAL_MAX_HUNDREDTHS) )
	{
		VitalValue& vv = chr->vital(vital->vitalid);
		chr->set_current(vital, vv, value);
		return new BLong(1);
	}
	else
		return new BError("Invalid parameter type");
}

BObjectImp* VitalExecutorModule::mf_ConsumeVital( /* mob, vital, hundredths */ )
{
	Character* chr;
	const Vital* vital;
	long hundredths;

	if ( getCharacterParam(exec, 0, chr) &&
		getVitalParam(exec, 1, vital) &&
		getParam(2, hundredths, VITAL_MAX_HUNDREDTHS) )
	{
		VitalValue& vv = chr->vital(vital->vitalid);
		bool res = chr->consume(vital, vv, hundredths);
		return new BLong(res?1:0);
	}
	else
		return new BError("Invalid parameter type");
}

BObjectImp* VitalExecutorModule::mf_RecalcVitals( /* mob, attributes, vitals */ )
{
	Character* chr;
	BObjectImp* param1 = getParamImp(1);
	BObjectImp* param2 = getParamImp(2);

	if ( getCharacterParam(exec, 0, chr) && param1 && param2 )
	{
		if ( chr->logged_in )
		{
            bool calc_attr=false;
            bool calc_vital=false;

            if (param1->isa( BObjectImp::OTLong ))
                calc_attr = static_cast<BLong*>(param1)->isTrue();
            else if (param1->isa( BObjectImp::OTString ))
            {
                String* attrname = static_cast<String*>(param1);
                Attribute* attr = FindAttribute(attrname->value());
                if (!attr)
					return new BError( "Attribute not defined: " + attrname->value() );
                chr->calc_single_attribute( attr );
            }
            else
                return new BError("Invalid parameter type");

            if (param2->isa( BObjectImp::OTLong ))
                calc_vital = static_cast<BLong*>(param2)->isTrue();
            else if (param2->isa( BObjectImp::OTString ))
            {
                String* vitalname = static_cast<String*>(param2);
				Vital* vital = FindVital( vitalname->value() );
				if (!vital)
					return new BError( "Vital not defined: " + vitalname->value() );
                chr->calc_single_vital( vital );
            }
            else
                return new BError("Invalid parameter type");

            chr->calc_vital_stuff( calc_attr, calc_vital );
			return new BLong(1);
		}
		else
			return new BError("Mobile must be online.");
	}
	else
		return new BError("Invalid parameter type");
}
