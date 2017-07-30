/** @file
 *
 * @par History
 */


#ifndef H_UNIEMOD_H
#define H_UNIEMOD_H

#ifndef BSCRIPT_EXECMODL_H
#include "../../bscript/execmodl.h"
#endif

#include "../pktdef.h"
#include "../uoexec.h"
#include "../uoexhelp.h"
namespace Pol
{
namespace Module
{
extern u16 gwtext[( SPEECH_MAX_LEN + 1 )];

class UnicodeExecutorModule : public Bscript::TmplExecutorModule<UnicodeExecutorModule>
{
public:
  explicit UnicodeExecutorModule( Core::UOExecutor& exec );
  ~UnicodeExecutorModule();

  Core::UOExecutor& uoexec;
  
  Bscript::BObjectImp* mf_PrintTextAboveUC();    // OverObject, Text, Font, Color
  Bscript::BObjectImp* mf_PrivateTextAboveUC();  // OverObject, Text, ToChar, Font, Color

  Bscript::BObjectImp* mf_BroadcastUC();       // Text
  Bscript::BObjectImp* mf_SendSysMessageUC();  // Character, Text

  Bscript::BObjectImp* mf_RequestInputUC();       // ToChar, Item, TextPrompt
  Bscript::BObjectImp* mf_SendTextEntryGumpUC();  // ToChar, Text1, Cancel, Style, MaxLen, Text2

  Mobile::Character* prompt_chr;

protected:
  bool getCharacterParam( unsigned param, Mobile::Character*& chrptr );
  bool getItemParam( unsigned param, Items::Item*& itemptr );
  bool getUObjectParam( unsigned param, Core::UObject*& objptr );
};

inline bool UnicodeExecutorModule::getCharacterParam( unsigned param, Mobile::Character*& chrptr )
{
  return Core::getCharacterParam( exec, param, chrptr );
}

inline bool UnicodeExecutorModule::getItemParam( unsigned param, Items::Item*& itemptr )
{
  return Core::getItemParam( exec, param, itemptr );
}

inline bool UnicodeExecutorModule::getUObjectParam( unsigned param, Core::UObject*& objptr )
{
  return Core::getUObjectParam( exec, param, objptr );
}
}
}
#endif
