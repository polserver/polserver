/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 */


#include <stdlib.h>
#include <string>

#include "bscript/eprog.h"
#include "clib/refptr.h"
#include "plib/systemstate.h"

#include "globals/network.h"
#include "mobile/attribute.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/pktin.h"
#include "polclock.h"
#include "scrdef.h"
#include "scrstore.h"
#include "skills.h"
#include "ufunc.h"
#include "uoskills.h"


namespace Pol::Core
{
void handle_use_skill( Network::Client* client, PKTIN_12* msg )
{
  char* params;
  unsigned int skillnum = (int)strtoul( (char*)msg->data, &params, 10 );

  if ( skillnum > networkManager.uoclient_general.maxskills )
    return;

  const UOSkill& uoskill = GetUOSkill( skillnum );
  if ( !uoskill.inited )
    return;
  const Mobile::Attribute* attrib = uoskill.pAttr;

  if ( !attrib->disable_core_checks && !CanUseSkill( client ) )
    return;
  else if ( !attrib->script_.empty() )
  {
    if ( StartSkillScript( client, attrib ) )
      return;
  }

  /*Log("Client#%lu: (%s, acct:%s) No handler for skill %d\n",
          client->instance_, client->chr->name().c_str(), client->acct->name(), int(skillnum));
          cerr << "Character " << client->chr->name() << " (acct: " << client->chr->acct->name() <<
     "): No handler for skill " << int(skillnum) << endl;*/
  send_sysmessage( client, "That skill cannot be used directly." );
}


bool StartSkillScript( Network::Client* client, const Mobile::Attribute* attrib )
{
  Mobile::Character* chr = client->chr;
  ref_ptr<Bscript::EScriptProgram> prog = find_script2(
      attrib->script_, true,
      /* complain if not found */ Plib::systemstate.config.cache_interactive_scripts );

  if ( prog.get() != nullptr )
  {
    if ( chr->start_skill_script( prog.get() ) )
    {
      // Should the script handle the unhiding instead?
      if ( chr->hidden() && attrib->unhides )
        chr->unhide();
      if ( attrib->delay_seconds )
      {
        chr->disable_skills_until( poltime() + attrib->delay_seconds );
      }
      return true;
    }
  }
  std::string msg = "Unable to start skill script:";  //+attrib->script_.c_str();
  msg += attrib->script_.c_str();
  send_sysmessage( client, msg.c_str() );

  return false;
}

bool CanUseSkill( Network::Client* client )
{
  Mobile::Character* chr = client->chr;

  if ( chr->dead() )
  {
    private_say_above( chr, chr, "I am dead and cannot do that." );
    return false;
  }
  else if ( chr->skill_ex_active() || chr->casting_spell() )
  {
    private_say_above( chr, chr, "I am already performing another action." );
    return false;
  }
  else if ( poltime() < chr->disable_skills_until() )
  {
    send_sysmessage( client, "You must wait to perform another action." );
    return false;
  }
  else if ( chr->frozen() )
  {
    private_say_above( chr, chr, "I am frozen and cannot do that." );
    return false;
  }
  else if ( chr->paralyzed() )
  {
    private_say_above( chr, chr, "I am paralyzed and cannot do that." );
    return false;
  }

  return true;
}
}  // namespace Pol::Core
