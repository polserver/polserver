/** @file
 *
 * @par History
 * - 2009/11/24 Turley:    Added realm check
 */


#include "listenpt.h"

#include <stddef.h>

#include "../bscript/bobject.h"
#include "globals/settings.h"
#include "globals/uvars.h"
#include "mobile/charactr.h"
#include "module/osmod.h"
#include "uconst.h"
#include "ufunc.h"
#include "uoexec.h"
#include "uoscrobj.h"

namespace Pol
{
namespace Core
{
ListenPoint::ListenPoint( UObject* obj, UOExecutor* uoexec, int range, int flags )
    : object( obj ), uoexec( uoexec ), range( range ), flags( flags )
{
}

ListenPoint::~ListenPoint() {}

const char* TextTypeToString( u8 texttype )
{
  switch ( texttype )
  {
  case TEXTTYPE_WHISPER:
    return "whisper";
  case TEXTTYPE_YELL:
    return "yell";
  case TEXTTYPE_EMOTE:
    return "emote";
  default:
    return "default";
  }
}

void sayto_listening_points( Mobile::Character* speaker, const char* p_text, int /*p_textlen*/,
                             u8 texttype,  // DAVE
                             const Clib::UnicodeString* p_wtext /*=nullptr*/,
                             const char* p_lang /*=nullptr*/,
                             Bscript::ObjArray* speechtokens /*=nullptr*/ )
{
  for ( ListenPoints::iterator itr = gamestate.listen_points.begin(),
                               end = gamestate.listen_points.end();
        itr != end; )
  {
    ListenPoint* lp = ( *itr ).second;
    if ( lp->object->orphan() )
    {
      ListenPoints::iterator next = itr;
      ++next;
      gamestate.listen_points.erase( itr );
      delete lp;
      itr = next;
      end = gamestate.listen_points.end();
    }
    else
    {
      if ( !speaker->dead() || ( lp->flags & LISTENPT_HEAR_GHOSTS ) )
      {
        if ( settingsManager.ssopt.seperate_speechtoken )
        {
          if ( speechtokens != NULL && ( ( lp->flags & LISTENPT_HEAR_TOKENS ) == 0 ) )
          {
            ++itr;
            continue;
          }
          else if ( speechtokens == NULL && ( lp->flags & LISTENPT_NO_SPEECH ) )
          {
            ++itr;
            continue;
          }
        }
        const UObject* toplevel = lp->object->toplevel_owner();
        if ( ( speaker->realm == toplevel->realm ) &&
             ( inrangex( speaker, toplevel->x, toplevel->y, lp->range ) ) )
        {
          if ( p_wtext && p_lang && p_wtext->lengthc() > 0 )
            lp->uoexec->os_module->signal_event( new Module::UnicodeSpeechEvent(
                speaker, p_text, TextTypeToString( texttype ), *p_wtext, p_lang, speechtokens ) );
          else
            lp->uoexec->os_module->signal_event(
                new Module::SpeechEvent( speaker, p_text, TextTypeToString( texttype ) ) );
        }
      }
      ++itr;
    }
  }
}

void deregister_from_speech_events( UOExecutor* uoexec )
{
  // ListenPoint lp( NULL, uoexec, 0, 0 );
  ListenPoints::iterator itr = gamestate.listen_points.find( uoexec );
  if ( itr !=
       gamestate.listen_points.end() )  // could have been cleaned up in sayto_listening_points
  {
    ListenPoint* lp = ( *itr ).second;
    gamestate.listen_points.erase( uoexec );
    delete lp;
  }
}

void register_for_speech_events( UObject* obj, UOExecutor* uoexec, int range, int flags )
{
  gamestate.listen_points[uoexec] = new ListenPoint( obj, uoexec, range, flags );
}

Bscript::BObjectImp* GetListenPoints()
{
  Bscript::ObjArray* arr = new Bscript::ObjArray;
  for ( ListenPoints::iterator itr = gamestate.listen_points.begin(),
                               end = gamestate.listen_points.end();
        itr != end; )
  {
    ListenPoint* lp = ( *itr ).second;
    if ( lp == NULL || lp->object->orphan() )
    {
      ListenPoints::iterator next = itr;
      ++next;
      gamestate.listen_points.erase( itr );
      delete lp;
      itr = next;
      end = gamestate.listen_points.end();
    }
    else
    {
      arr->addElement( lp->object->make_ref() );

      ++itr;
    }
  }
  return arr;
}
}
}
