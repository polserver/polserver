const DAMAGE_USE_REPSYS := 1;
const DAMAGE_NO_REPSYS := 0;
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
RecalcVitals( character );
SetVital( character, vitalname, value );
