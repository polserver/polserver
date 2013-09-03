/*
History
=======
2009/11/24 Turley:    Added realm check


Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/refptr.h"

#include "mobile/charactr.h"
#include "module/osmod.h"
#include "uobject.h"
#include "ufunc.h"
#include "uoexec.h"
#include "uoscrobj.h"
#include "listenpt.h"

class UOExecutor;
class ListenPoint 
{
public:
    ListenPoint( UObject* obj, UOExecutor* uoexec, int range, int flags );
    ~ListenPoint();

    UObjectRef object;
    UOExecutor* uoexec;
    int range;
    int flags;
};

ListenPoint::ListenPoint( UObject* obj, UOExecutor* uoexec, int range, int flags ) :
    object(obj),
    uoexec(uoexec),
    range(range),
    flags(flags)
{
}

ListenPoint::~ListenPoint()
{
}


typedef map<UOExecutor*, ListenPoint*> ListenPoints;
ListenPoints listen_points;

//DAVE
const char* TextTypeToString(u8 texttype)
{
	switch(texttype)
	{
	case TEXTTYPE_WHISPER: return "whisper";
	case TEXTTYPE_YELL: return "yell";
	case TEXTTYPE_EMOTE: return "emote";
	default: return "default";
	}
}

void sayto_listening_points( Character* speaker, const char* p_text, int p_textlen, u8 texttype, //DAVE
							const u16* p_wtext /*NULL*/, const char* p_lang /*NULL*/,
							int p_wtextlen /*0*/, ObjArray* speechtokens /*NULL*/)
{
    for( ListenPoints::iterator itr = listen_points.begin(), end = listen_points.end(); itr != end; )
    {
        ListenPoint* lp = (*itr).second;
        if (lp->object->orphan())
        { 
            ListenPoints::iterator next = itr;
            ++next;
            listen_points.erase( itr );
            delete lp;
            itr = next;
            end = listen_points.end();
        }
        else
        { 
            if (!speaker->dead() || (lp->flags&LISTENPT_HEAR_GHOSTS))
            {
				if (ssopt.seperate_speechtoken)
				{
					if (speechtokens != NULL && ((lp->flags & LISTENPT_HEAR_TOKENS)==0))
					{
						++itr;
						continue;
					}
					else if (speechtokens == NULL && (lp->flags & LISTENPT_NO_SPEECH))
					{
						++itr;
						continue;
					}
				}
                const UObject* toplevel = lp->object->toplevel_owner();
                if ((speaker->realm == toplevel->realm) && (inrangex( speaker, toplevel->x, toplevel->y, lp->range )))
                {
					if ( p_wtext && p_lang && p_wtextlen > 0 )
						lp->uoexec->os_module->signal_event( new UnicodeSpeechEvent(speaker,p_text,
																			 TextTypeToString(texttype),
																			 p_wtext,p_lang, speechtokens) );
					else
	                    lp->uoexec->os_module->signal_event( new SpeechEvent(speaker, p_text,
																			 TextTypeToString(texttype)) );
                }
            }
            ++itr;
        }
    }
}

void deregister_from_speech_events( UOExecutor* uoexec )
{
    //ListenPoint lp( NULL, uoexec, 0, 0 );
    ListenPoints::iterator itr = listen_points.find( uoexec );
    if (itr != listen_points.end() ) // could have been cleaned up in sayto_listening_points
    {
        ListenPoint* lp = (*itr).second;
        listen_points.erase( uoexec );
        delete lp;
    }
}

void register_for_speech_events( UObject* obj, UOExecutor* uoexec, int range, int flags )
{
    listen_points[ uoexec ] = new ListenPoint(obj, uoexec, range, flags);
}

void clear_listen_points()
{
    for( ListenPoints::iterator itr = listen_points.begin(); itr != listen_points.end(); ++itr )
    {
        ListenPoint* lp = (*itr).second;
        delete lp;
        (*itr).second = NULL;
    }
    listen_points.clear();
}

BObjectImp* GetListenPoints()
{
    ObjArray* arr = new ObjArray;
    for( ListenPoints::iterator itr = listen_points.begin(), end = listen_points.end(); itr != end; )
    {
        ListenPoint* lp = (*itr).second;
        if (lp==NULL || lp->object->orphan())
        { 
            ListenPoints::iterator next = itr;
            ++next;
            listen_points.erase( itr );
            delete lp;
            itr = next;
            end = listen_points.end();
        }
        else
        {
            arr->addElement( lp->object->make_ref() );

            ++itr;
        }
    }
    return arr;
}
