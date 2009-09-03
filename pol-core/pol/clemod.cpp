/*
History
=======
2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule

Notes
=======

*/

#include "clib/stl_inc.h"
#include "clib/unicode.h"

#include "bscript/bobject.h"
#include "bscript/berror.h"

#include "charactr.h"
#include "client.h"
#include "uoexhelp.h"

#include "clemod.h"
#include "clfunc.h"

template<>
TmplExecutorModule<ClilocExecutorModule>::FunctionDef
	TmplExecutorModule<ClilocExecutorModule>::function_table[] =
{

	{ "SendSysMessageCL",				&ClilocExecutorModule::mf_SendSysMessageCL },
	{ "PrintTextAboveCL",				&ClilocExecutorModule::mf_PrintTextAboveCL },
	{ "PrintTextAbovePrivateCL",		&ClilocExecutorModule::mf_PrintTextAbovePrivateCL }

};
template<>
int TmplExecutorModule<ClilocExecutorModule>::function_table_size =
	arsize(function_table);

BObjectImp* ClilocExecutorModule::mf_SendSysMessageCL()
{
    Character* chr;
	ObjArray* oText;
	unsigned int cliloc_num;
	unsigned short color;
    unsigned short font;

    if (getCharacterParam( exec, 0, chr ) &&
        getParam( 1, cliloc_num ) &&
		getObjArrayParam( 2, oText ) &&
		getParam( 3, font ) &&
        getParam( 4, color ))
    {
        if (chr->has_active_client())
        {
			u16 cltext[ (SPEECH_MAX_LEN + 1) ];
			if ( oText != NULL  )
			{
				if ( oText->ref_arr.size() > SPEECH_MAX_LEN )
					return new BError( "Unicode array exceeds maximum size." );
				unsigned textlen = oText->ref_arr.size();
				if ( !convertArrayToUC(oText, cltext, textlen, false) )
					return new BError( "Invalid value in Unicode array." );
			}
			send_sysmessage_cl( chr->client, cliloc_num, cltext, font, color);
			return new BLong(1);
        }
        else
        {
            return new BError( "Mobile has no active client" );
        }
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}

BObjectImp* ClilocExecutorModule::mf_PrintTextAboveCL()
{
    UObject* obj;
	ObjArray* oText;
	unsigned int cliloc_num;
	unsigned short color;
    unsigned short font;

    if (getUObjectParam( exec, 0, obj ) &&
        getParam( 1, cliloc_num ) &&
		getObjArrayParam( 2, oText ) &&
		getParam( 3, font ) &&
        getParam( 4, color ))
    {
		u16 cltext[ (SPEECH_MAX_LEN + 1) ];
		if ( oText != NULL  )
		{
			if ( oText->ref_arr.size() > SPEECH_MAX_LEN )
				return new BError( "Unicode array exceeds maximum size." );
			unsigned textlen = oText->ref_arr.size();
			if ( !convertArrayToUC(oText, cltext, textlen, false) )
				return new BError( "Invalid value in Unicode array." );
		}
		say_above_cl( obj, cliloc_num, cltext, font, color);
		return new BLong(1);
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}

BObjectImp* ClilocExecutorModule::mf_PrintTextAbovePrivateCL()
{
    Character* chr;
	UObject* obj;
	ObjArray* oText;
	unsigned int cliloc_num;
	unsigned short color;
    unsigned short font;

    if (getCharacterParam( exec, 0, chr ) &&
		getUObjectParam( exec, 1, obj ) &&
        getParam( 2, cliloc_num ) &&
		getObjArrayParam( 3, oText ) &&
		getParam( 4, font ) &&
        getParam( 5, color ))
    {
        if (chr->has_active_client())
        {
			if (chr->realm != obj->realm)
				return new BError( "Cannot print messages across realms!" );

			u16 cltext[ (SPEECH_MAX_LEN + 1) ];
			if ( oText != NULL  )
			{
				if ( oText->ref_arr.size() > SPEECH_MAX_LEN )
					return new BError( "Unicode array exceeds maximum size." );
				unsigned textlen = oText->ref_arr.size();
				if ( !convertArrayToUC(oText, cltext, textlen, false) )
					return new BError( "Invalid value in Unicode array." );
			}
			private_say_above_cl( chr, obj, cliloc_num, cltext, font, color);
			return new BLong(1);
        }
        else
        {
            return new BError( "Mobile has no active client" );
        }
    }
    else
    {
        return new BError( "Invalid parameter type" );
    }
}
