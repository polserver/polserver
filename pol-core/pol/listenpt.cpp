/** @file
 *
 * @par History
 * - 2009/11/24 Turley:    Added realm check
 */


#include "listenpt.h"

#include <algorithm>
#include <limits>
#include <stddef.h>

#include "../bscript/bobject.h"
#include "../plib/uconst.h"
#include "globals/settings.h"
#include "globals/uvars.h"
#include "mobile/charactr.h"
#include "uoexec.h"
#include "uoscrobj.h"

namespace Pol
{
namespace Core
{
ListenPoint::ListenPoint( UObject* obj, UOExecutor* uoexec, u16 range, int flags )
    : object( obj ), uoexec( uoexec ), range( range ), flags( flags )
{
}

std::string ListenPoint::TextTypeToString( u8 texttype )
{
  switch ( texttype )
  {
  case Plib::TEXTTYPE_WHISPER:
    return "whisper";
  case Plib::TEXTTYPE_YELL:
    return "yell";
  case Plib::TEXTTYPE_EMOTE:
    return "emote";
  default:
    return "default";
  }
}

void ListenPoint::sayto_listening_points( Mobile::Character* speaker, const std::string& text,
                                          u8 texttype, const char* p_lang,
                                          Bscript::ObjArray* speechtokens )
{
  ListenPoints::iterator itr = gamestate.listen_points.begin();
  while ( itr != gamestate.listen_points.end() )
  {
    ListenPoint* lp = itr->second;
    if ( lp->object->orphan() )
    {
      itr = gamestate.listen_points.erase( itr );
      delete lp;
      continue;
    }
    ++itr;
    lp->sayto( speaker, text, texttype, p_lang, speechtokens );
  }
}

void ListenPoint::sayto( Mobile::Character* speaker, const std::string& text, u8 texttype,
                         const char* p_lang, Bscript::ObjArray* speechtokens ) const
{
  if ( speaker->dead() && !( flags & LISTENPT_HEAR_GHOSTS ) )
    return;
  if ( settingsManager.ssopt.seperate_speechtoken )
  {
    if ( speechtokens != nullptr && ( ( flags & LISTENPT_HEAR_TOKENS ) == 0 ) )
      return;
    else if ( speechtokens == nullptr && ( flags & LISTENPT_NO_SPEECH ) )
      return;
  }
  if ( !speaker->in_range( object.get(), range ) )
    return;
  if ( p_lang )
    uoexec->signal_event( new Module::SpeechEvent( speaker, text, TextTypeToString( texttype ),
                                                   p_lang, speechtokens ) );
  else
    uoexec->signal_event( new Module::SpeechEvent( speaker, text, TextTypeToString( texttype ) ) );
}

void ListenPoint::deregister_from_speech_events( UOExecutor* uoexec )
{
  // ListenPoint lp( nullptr, uoexec, 0, 0 );
  ListenPoints::iterator itr = gamestate.listen_points.find( uoexec );
  if ( itr !=
       gamestate.listen_points.end() )  // could have been cleaned up in sayto_listening_points
  {
    ListenPoint* lp = ( *itr ).second;
    gamestate.listen_points.erase( uoexec );
    delete lp;
  }
}

void ListenPoint::register_for_speech_events( UObject* obj, UOExecutor* uoexec, int range,
                                              int flags )
{
  gamestate.listen_points[uoexec] =
      new ListenPoint( obj, uoexec,
                       static_cast<u16>( std::clamp(
                           range, 0, static_cast<int>( std::numeric_limits<u16>::max() ) ) ),
                       flags );
}

Bscript::BObjectImp* ListenPoint::GetListenPoints()
{
  Bscript::ObjArray* arr = new Bscript::ObjArray;
  for ( ListenPoints::iterator itr = gamestate.listen_points.begin(),
                               end = gamestate.listen_points.end();
        itr != end; )
  {
    ListenPoint* lp = ( *itr ).second;
    if ( lp == nullptr || lp->object->orphan() )
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
}  // namespace Core
}  // namespace Pol
