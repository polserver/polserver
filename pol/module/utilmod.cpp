/*
History
=======
2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
2008/07/08 Turley:    Added mf_RandomIntMinMax - Return Random Value between...

Notes
=======

*/

#include <time.h>

#include "../../clib/stl_inc.h"
#include "../../clib/random.h"
#include "../../clib/rawtypes.h"

#include "../../bscript/berror.h"
#include "../../bscript/execmodl.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"

#include "../dice.h"

#include "utilmod.h"


template<>
TmplExecutorModule<UtilExecutorModule>::FunctionDef
    TmplExecutorModule<UtilExecutorModule>::function_table[] =
{
    { "RandomInt",      &UtilExecutorModule::mf_RandomInt },
    { "RandomFloat",    &UtilExecutorModule::mf_RandomFloat },
    { "RandomDiceRoll", &UtilExecutorModule::mf_RandomDiceRoll },
	{ "StrFormatTime", &UtilExecutorModule::mf_StrFormatTime },
	{ "RandomIntMinMax", &UtilExecutorModule::mf_RandomIntMinMax}
};

template<>
int TmplExecutorModule<UtilExecutorModule>::function_table_size =
    arsize(function_table);


BObjectImp* UtilExecutorModule::mf_RandomInt()
{
    long value;
    if (exec.getParam( 0, value, 1, LONG_MAX ))
    {
        if (value > 0)
            return new BLong( random_int( value ) );
        else
            return new BError( "RandomInt() expects a positive integer" );
    }
    else
    {
        return new BError( "RandomInt() expects a positive integer" );
    }
}

BObjectImp* UtilExecutorModule::mf_RandomIntMinMax()
{
    long minvalue;
    if (exec.getParam( 0, minvalue, LONG_MIN, LONG_MAX ))
    {
        long maxvalue;
		if (exec.getParam( 1, maxvalue, LONG_MIN, LONG_MAX ))
		{
			if (maxvalue<minvalue)
				swap(maxvalue,minvalue);
			return new BLong( random_int_range( minvalue,maxvalue ) );
		}
		else
			return new BError( "RandomIntMinMax() expects an integer" );
    }
    else
        return new BError( "RandomIntMinMax() expects an integer" );
}

BObjectImp* UtilExecutorModule::mf_RandomFloat()
{
    double value;
    if (exec.getRealParam( 0, value ))
    {
        return new Double( static_cast<double>(random_float( static_cast<float>(value) )) );
    }
    else
    {
        return new BError( "RandomFloat() expects a Real parameter" );
    }
}

BObjectImp* UtilExecutorModule::mf_RandomDiceRoll()
{
    const String* dicestr;
    if (exec.getStringParam( 0, dicestr ))
    {
        string errormsg;
        Dice dice;
        if (dice.load( dicestr->data(), &errormsg ))
        {
            return new BLong( dice.roll() );
        }
        else
        {
            return new BError( errormsg.c_str() );
        }
    }
    else
    {
        return new BError( "RandomDiceRoll() expects a String as parameter" );
    }
}

BObjectImp* UtilExecutorModule::mf_StrFormatTime()
{
	const String* format_string;
	getStringParam(0, format_string);
	if ( !getStringParam(0, format_string) )
		return new BError("No time string passed.");
	else if ( format_string->length() > 100 )
		return new BError("Format string exceeded 100 characters.");

	long time_stamp;
	getParam(1, time_stamp, 0, LONG_MAX);
		
	time_t seconds;
	struct tm* time_struct;

	if ( time_stamp <= 0 )
		seconds = time(NULL);
	else
		seconds = time_stamp;

	time_struct = localtime(&seconds);

	char buffer[102]; // +2 for the \0 termination.
	strftime(buffer, sizeof buffer, format_string->data(), time_struct);

	return new String(buffer);
}

