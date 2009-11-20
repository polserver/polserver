//Apply*Damage constants
const DAMAGE_USE_REPSYS := 1;
const DAMAGE_NO_REPSYS := 0;

//RecalcVital constants
const RECALC_INTRINSIC_MOD    := 1;
const RECALC_NO_INTRINSIC_MOD := 0;
const RECALC_VITALS           := 1;
const RECALC_NO_VITALS        := 0;

//
// Vital related functions
//
ApplyDamage( mobile, damage, userepsys := DAMAGE_USE_REPSYS );
ApplyRawDamage( character, hits, userepsys := DAMAGE_USE_REPSYS );
HealDamage( character, hits );
ConsumeMana( who, spellid );

ConsumeVital( who, vital, hundredths );
GetVitalName( alias_name );
GetVital( character, vitalname );
GetVitalMaximumValue( character, vitalname );
GetVitalRegenRate( character, vitalname );
RecalcVitals( character, calc_attribute := RECALC_INTRINSIC_MOD, calc_vital := RECALC_VITALS );
SetVital( character, vitalname, value );
