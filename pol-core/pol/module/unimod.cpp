/*
History
=======
2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
2009/09/03 MuadDib:   Relocation of account related cpp/h

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning(disable:4786)
#endif

#include "../../clib/endian.h"
#include "../../clib/logfile.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"
#include "../../clib/unicode.h"

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/execmodl.h"
#include "../../bscript/impstr.h"

#include "../accounts/account.h"
#include "../network/cgdata.h"
#include "../mobile/charactr.h"
#include "../network/client.h"
#include "../msghandl.h"
#include "../pktboth.h"
#include "../sockio.h"
#include "../ufunc.h"

#include "osmod.h"
#include "unimod.h"

//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

char gtext[ 2*1024 ];
u16 gwtext[ (SPEECH_MAX_LEN + 1) ];

void send_unicode_prompt( Client* client, u32 serial )
{
	PktHelper::PacketOut<PktOut_C2> msg;
	msg->WriteFlipped(static_cast<u16>(PKTBI_C2::SERVER_MSGLEN));
	msg->Write(serial); //serial
	msg->Write(serial); //msg_id  Server-"decided" message ID. o_O
	msg->offset+=10; // 10x u8 unk
	msg.Send(client);
}

void handle_unicode_prompt( Client* client, PKTBI_C2* msg )
{
    UnicodeExecutorModule* uniemod = client->gd->prompt_uniemod;
    if (uniemod == NULL)
        return; // log it?

	int textlen = ((cfBEu16(msg->msglen) - offsetof(PKTBI_C2,wtext))
					/ sizeof(msg->wtext[0])); //note NO terminator!
	if (textlen < 0)
		textlen = 0;
	if (textlen > SPEECH_MAX_LEN)
	{
		textlen = SPEECH_MAX_LEN;	// ENHANCE: May want to log this
		msg->wtext[textlen] = 0x0000;
	}

	bool ok = true;
	BObject* valstack = NULL;

#if(0)
	// client version of the packet should always send this as a 1??
	if (cfBEu16(msg->unk) != 0x01)
	{
		// ENHANCE: May want to log this, too?
		ok = false;
		valstack = new BObject( new BError("Malformed return-packet from client") );
	}
#endif

	char lang[4];
	memcpy(lang, msg->lang, 3);
	lang[3] = 0x00;

	if ( ok )
	{
		ObjArray uc_text;
		int i;
		for ( i = 0; i < textlen; i++ )
		{
			u16 wc = cfBEu16(msg->wtext[i]);
			if ( wc < (u16)0x20 || wc == (u16)0x7F ) // control character! >_<
			{
				ok = false;
				valstack = new BObject( new BError( "Invalid control characters in text entry" ) );

				sprintf(gtext, "Client #%lu (account %s) sent invalid unicode control characters (RequestInputUC)",
								static_cast<unsigned long>(client->instance_),
								(client->acct != NULL) ? client->acct->name() : "unknown");
				cerr << gtext << endl;
				Log("%s\n", gtext);
				break; //for
			}
			uc_text.addElement( new BLong( wc ) );
		}

		if (ok)
		{
			if ( uc_text.ref_arr.empty() )
				valstack = new BObject( new BLong( 0 ) );
			else
			{
				auto_ptr<BStruct> retval (new BStruct());
				retval->addMember("lang", new String( lang ) );
				retval->addMember("uc_text", uc_text.copy() );
				valstack = new BObject( retval.release() );
			}
		}
	}

	uniemod->exec.ValueStack.top().set( valstack ); // error or struct, regardless.
	uniemod->os_module->revive();
    uniemod->prompt_chr = NULL;
    client->gd->prompt_uniemod = NULL;
}
MESSAGE_HANDLER_VARLEN( PKTBI_C2, handle_unicode_prompt );

//////////////////////////////////////////////////////////////////////////

template<>
TmplExecutorModule<UnicodeExecutorModule>::FunctionDef
    TmplExecutorModule<UnicodeExecutorModule>::function_table[] =
{
    { "BroadcastUC",				&UnicodeExecutorModule::mf_BroadcastUC },
    { "PrintTextAboveUC",			&UnicodeExecutorModule::mf_PrintTextAboveUC },
    { "PrintTextAbovePrivateUC",	&UnicodeExecutorModule::mf_PrivateTextAboveUC },
	{ "RequestInputUC",				&UnicodeExecutorModule::mf_RequestInputUC },
	{ "SendSysMessageUC",			&UnicodeExecutorModule::mf_SendSysMessageUC },
	{ "SendTextEntryGumpUC",		&UnicodeExecutorModule::mf_SendTextEntryGumpUC }
};
template<>
int TmplExecutorModule<UnicodeExecutorModule>::function_table_size =
    arsize(function_table);

UnicodeExecutorModule::UnicodeExecutorModule( Executor& exec ) :
	TmplExecutorModule<UnicodeExecutorModule>( "unicode", exec ),
	prompt_chr(NULL)
{
	os_module = static_cast<OSExecutorModule*>(exec.findModule("OS"));
	if (os_module == NULL)
		throw runtime_error( "UnicodeExecutorModule needs OS module!" );
}

UnicodeExecutorModule::~UnicodeExecutorModule()
{
    if (prompt_chr != NULL)
    {
        prompt_chr->client->gd->prompt_uniemod = NULL;
        prompt_chr = NULL;
    }
}

BObjectImp* UnicodeExecutorModule::mf_BroadcastUC()
{
using std::wcout; // wcout rox :)

	ObjArray* oText;
	const String* lang;
    unsigned short font;
    unsigned short color;
	if (getObjArrayParam( 0, oText ) &&
		getStringParam( 1, lang ) &&
        getParam( 2, font ) && // todo: getFontParam
        getParam( 3, color ))   // todo: getColorParam
    {
		size_t textlen = oText->ref_arr.size();
		if ( textlen > SPEECH_MAX_LEN )
			return new BError( "Unicode array exceeds maximum size." );
		if ( lang->length() != 3 )
			return new BError( "langcode must be a 3-character code." );
		//lang->toUpper(); // Language codes are in upper-case :)
		if ( !convertArrayToUC(oText, gwtext, textlen) )
			return new BError( "Invalid value in Unicode array." );
		::broadcast( gwtext, strupper(lang->value()).c_str(), font, color );
        return new BLong( 1 );
    }
    else
    {
        return new BError( "A parameter was invalid" );
    }
}

BObjectImp* UnicodeExecutorModule::mf_PrintTextAboveUC()
{
    UObject* obj;
	ObjArray* oText;
	const String* lang;
    unsigned short font;
    unsigned short color;
	int journal_print;

    if (getUObjectParam( 0, obj ) &&
        getObjArrayParam( 1, oText ) &&
		getStringParam( 2, lang ) &&
        getParam( 3, font ) &&
        getParam( 4, color ) &&
		getParam( 5, journal_print) )
    {
		size_t textlen = oText->ref_arr.size();
		if ( textlen > SPEECH_MAX_LEN )
			return new BError( "Unicode array exceeds maximum size." );
		if ( lang->length() != 3 )
			return new BError( "langcode must be a 3-character code." );
		if ( !convertArrayToUC(oText, gwtext, textlen) )
			return new BError( "Invalid value in Unicode array." );

		return new BLong( say_above( obj, gwtext, strupper(lang->value()).c_str(), font, color, journal_print ) );
    }
    else
    {
        return new BError( "A parameter was invalid" );
    }
}

BObjectImp* UnicodeExecutorModule::mf_PrivateTextAboveUC()
{
    Character* chr;
    UObject* obj;
    ObjArray* oText;
	const String* lang;
    unsigned short font;
    unsigned short color;

    if (getUObjectParam( 0, obj ) &&
        getObjArrayParam( 1, oText ) &&
		getStringParam( 2, lang ) &&
        getCharacterParam( 3, chr ) &&
        getParam( 4, font ) &&
        getParam( 5, color ) )
    {
		size_t textlen = oText->ref_arr.size();
		if ( textlen > SPEECH_MAX_LEN )
			return new BError( "Unicode array exceeds maximum size." );
		if ( lang->length() != 3 )
			return new BError( "langcode must be a 3-character code." );
		//lang->toUpper(); // Language codes are in upper-case :)
		if ( !convertArrayToUC(oText, gwtext, textlen) )
			return new BError( "Invalid value in Unicode array." );

		return new BLong( private_say_above( chr, obj, gwtext, strupper(lang->value()).c_str(), font, color ) );
    }
    else
    {
        return new BError( "A parameter was invalid" );
    }
}

BObjectImp* UnicodeExecutorModule::mf_RequestInputUC()
{
    Character* chr;
    Item* item;
    ObjArray* oPrompt;
	const String *lang;
    if (getCharacterParam( 0, chr ) &&
        getItemParam( 1, item ) &&
        getObjArrayParam( 2, oPrompt ) &&
		getStringParam( 3, lang ))
    {
	    if (!chr->has_active_client())
	    {
			return new BError( "No client attached" );
		}

		if (chr->has_active_prompt())
		{
	        return new BError( "Another script has an active prompt" );
	    }

		size_t textlen = oPrompt->ref_arr.size();
		if ( textlen > SPEECH_MAX_LEN )
			return new BError( "Unicode array exceeds maximum size." );
		if ( lang->length() != 3 )
			return new BError( "langcode must be a 3-character code." );
		if ( !convertArrayToUC(oPrompt, gwtext, textlen) )
			return new BError( "Invalid value in Unicode array." );

	    send_sysmessage( chr->client, gwtext, strupper(lang->value()).c_str() );

		chr->client->gd->prompt_uniemod = this;
		prompt_chr = chr;

		send_unicode_prompt( chr->client, ctBEu32( item->serial ) );
		os_module->suspend();
		return new BLong(0);
	}
	else
	{
        return new BError( "A parameter was invalid" );
    }
}

BObjectImp* UnicodeExecutorModule::mf_SendSysMessageUC()
{
    Character* chr;
	ObjArray* oText;
	const String* lang;
    unsigned short font;
    unsigned short color;

    if (getCharacterParam( 0, chr ) &&
        getObjArrayParam( 1, oText ) &&
		getStringParam( 2, lang ) &&
        getParam( 3, font ) &&
        getParam( 4, color ))
    {
        if (chr->has_active_client())
        {
			size_t textlen = oText->ref_arr.size();
			if ( textlen > SPEECH_MAX_LEN )
				return new BError( "Unicode array exceeds maximum size." );
			if ( lang->length() != 3 )
				return new BError( "langcode must be a 3-character code." );
			//lang->toUpper(); // Language codes are in upper-case :)
			if ( !convertArrayToUC(oText, gwtext, textlen) )
				return new BError( "Invalid value in Unicode array." );

            send_sysmessage( chr->client, gwtext, strupper(lang->value()).c_str(), font, color );
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

BObjectImp* UnicodeExecutorModule::mf_SendTextEntryGumpUC()
{
	//SendTextEntryGumpUC(who, uc_text1, cancel := TE_CANCEL_ENABLE,
	//						style := TE_STYLE_NORMAL, maximum := 40, uc_text2 := {} );

	return new BError( "Function not implimented" );
}
