/*
History
=======


Notes
=======

*/

#ifndef LISTENPT_H
#define LISTENPT_H

class Character;
class UObject;
class UOExecutor;
class BObjectImp;

const char* TextTypeToString(u8 texttype); //DAVE

void sayto_listening_points( Character* speaker, const char* p_text, int p_textlen, u8 texttype, //DAVE
							 const u16* p_wtext = NULL, const char* p_lang = NULL, int p_wtextlen = 0, ObjArray* speechtokens = NULL);

const int LISTENPT_HEAR_GHOSTS = 0x01;
const int LISTENPT_HEAR_TOKENS = 0x02;
const int LISTENPT_NO_SPEECH = 0x04;

void register_for_speech_events( UObject* obj, UOExecutor* uoexec, int range, int flags );
void deregister_from_speech_events( UOExecutor* uoexec );

BObjectImp* GetListenPoints();

#endif
