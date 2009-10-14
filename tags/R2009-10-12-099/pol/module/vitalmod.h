/*
History
=======
2006/10/28 Shinigami: GCC 4.1.1 fix - extra qualification in class removed

Notes
=======

*/


#ifndef VITALEMOD_H
#define VITALEMOD_H

#include "../../bscript/execmodl.h"

#include "../mobile/charactr.h"

class VitalExecutorModule : public TmplExecutorModule<VitalExecutorModule>
{
public:
	VitalExecutorModule(Executor& exec) :
	  TmplExecutorModule<VitalExecutorModule>("vitals", exec) {}
	
	BObjectImp* mf_ApplyRawDamage();
    BObjectImp* mf_ApplyDamage();

	BObjectImp* mf_HealDamage();

	BObjectImp* mf_ConsumeMana();

	BObjectImp* mf_ConsumeVital( /* mob, vital, hundredths */);
	BObjectImp* mf_RecalcVitals( /* mob */ );

	BObjectImp* mf_GetVitalName(/*alias_name*/);
	
	BObjectImp* mf_GetVital( /* mob, vitalid */ );
	BObjectImp* mf_GetVitalMaximumValue( /* mob, vitalid */ );
	BObjectImp* mf_GetVitalRegenRate( /* mob, vitalid */ );

	BObjectImp* mf_SetVitalMaximumValue( /* mob, vitalid, value */ );
	BObjectImp* mf_SetVital( /* mob, vitalid, value */ );
	BObjectImp* mf_SetVitalRegenRate( /* mob, vitalid, rate */ );

private:
	Character* GetUOController();
};

inline Character* VitalExecutorModule::GetUOController()
{
	UOExecutorModule* uo_module = static_cast<UOExecutorModule*>(exec.findModule("UO"));

	if ( uo_module && uo_module->controller_.get() )
		return uo_module->controller_.get();
	else
		return NULL;
}

#endif
