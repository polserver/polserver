const UUID_NIL := "00000000-0000-0000-0000-000000000000";
const UUID_MAX := "ffffffff-ffff-ffff-ffff-ffffffffffff";

RandomInt( below_this_number ); // returns integer R such that 0 <= R < B
RandomIntMinMax( minValue, maxValue ); // returns integer R such that minValue<=R<maxValue
RandomFloat( below_this_float ); // returns float R such that 0.0 <= R < B
RandomUUID( version := 4 );

// formats for RandomDiceRoll dice_string parameter:
//      4
//      d4
//      d6+6
//      d6-2
//      2d12
//      2d8+12
//      2d8-4
// Returns an integer >= 0
RandomDiceRoll( dice_string, allow_negatives := 0 );

StrFormatTime( format_string, time_stamp := 0 );
