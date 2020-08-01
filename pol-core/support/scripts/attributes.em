//
// Attribute related functions
//

const ATTRIBUTE_LOCK_UP     := 0;
const ATTRIBUTE_LOCK_DOWN   := 1;
const ATTRIBUTE_LOCK_LOCKED := 2;

const ATTRIBUTE_PRECISION_NORMAL := 0;
const ATTRIBUTE_PRECISION_TENTHS := 1;

const ATTRIBUTE_MAX_BASE := 60000;

CheckSkill( character, skillid_or_attrname, difficulty, points );

AlterAttributeTemporaryMod( character, attrname, delta_tenths );

GetAttributeName( alias_name );

GetAttribute( character, attrname, precision := ATTRIBUTE_PRECISION_NORMAL );
GetAttributeBaseValue( character, attrname );
GetAttributeIntrinsicMod( character, attrname );
GetAttributeTemporaryMod( character, attrname );

GetAttributeLock( character, attrname );
SetAttributeLock( character, attrname, lockstate );

GetAttributeDefaultCap ( attrname );
GetAttributeCap( character, attrname );
SetAttributeCap( character, attrname, capvalue );

SetAttributeBaseValue( character, attrname, basevalue_tenths ); // obsoletes SetRawSkill
SetAttributeTemporaryMod( character, attrname, tempmod_tenths ); // obsoletes mob.strength_mod etc


BaseSkillToRawSkill( baseskill );
RawSkillToBaseSkill( rawskill );
