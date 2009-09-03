/*
History
=======


Notes
=======

*/

#ifndef H_UNIEMOD_H
#define H_UNIEMOD_H

#ifndef __BSCRIPT_EXECMODL_H
#include "bscript/execmodl.h"
#endif

#include "pktdef.h"
#include "uoexec.h"
#include "uoexhelp.h"

extern u16 gwtext[ (SPEECH_MAX_LEN + 1) ];

class UnicodeExecutorModule : public TmplExecutorModule<UnicodeExecutorModule>
{
public:
    explicit UnicodeExecutorModule( Executor& exec );
	~UnicodeExecutorModule();

	OSExecutorModule* os_module;

	BObjectImp* mf_PrintTextAboveUC();		// OverObject, Text, Font, Color
    BObjectImp* mf_PrivateTextAboveUC();	// OverObject, Text, ToChar, Font, Color

	BObjectImp* mf_BroadcastUC();			// Text
    BObjectImp* mf_SendSysMessageUC();		// Character, Text

	BObjectImp* mf_RequestInputUC();		// ToChar, Item, TextPrompt
	BObjectImp* mf_SendTextEntryGumpUC();	// ToChar, Text1, Cancel, Style, MaxLen, Text2

	Character* prompt_chr;

protected:
	bool getCharacterParam( unsigned param, Character*& chrptr );
	bool getItemParam( unsigned param, Item*& itemptr );
	bool getUObjectParam( unsigned param, UObject*& objptr );
};

inline bool UnicodeExecutorModule::getCharacterParam( unsigned param, Character*& chrptr )
{
    return ::getCharacterParam( exec, param, chrptr );
}

inline bool UnicodeExecutorModule::getItemParam( unsigned param, Item*& itemptr )
{
	return ::getItemParam( exec, param, itemptr );
}

inline bool UnicodeExecutorModule::getUObjectParam( unsigned param, UObject*& objptr )
{
    return ::getUObjectParam( exec, param, objptr );
}

#endif
