/*
History
=======
2005/12/05 MuadDib:   Added ssopt.invul_tag for Repsys::hilite_color_idx
2005/12/05 MuadDib:   Added ssopt.invul_tag for NPC::hilite_color_idx
2005/12/05 MuadDib:   Added repsys.invulnerable color for names.
2009/08/06 Turley:    Added escript interface to add/remove aggressor/lawfullydamaged

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/cfgsect.h"
#include "../clib/logfile.h"
#include "../clib/stlutil.h"

#include "cmbtcfg.h"
#include "mobile/charactr.h"
#include "fnsearch.h"
#include "module/guildmod.h"
#include "npc.h"
#include "npctmpl.h"
#include "polclock.h"
#include "polsig.h"
#include "repsys.h"
#include "schedule.h"
#include "ssopt.h"
#include "syshook.h"
#include "ufunc.h"
#include "uvars.h"
#include "uworld.h"

// BUGS:
//  it looks like you can restart someone's aggressor timer by toggling war mode and setting opponent.
//


///$TITLE=Reputation System

/// [1] Reputation System Configuration
/// Reputation System Configuration is read from config/repsys.cfg
/// This file is re-read on a config reload.
namespace Pol {
  namespace Core {
    struct RepSys_Cfg
    {
      struct
      {
        unsigned short Murderer;
        unsigned short Criminal;
        unsigned short Attackable;
        unsigned short Innocent;
        unsigned short GuildAlly;
        unsigned short GuildEnemy;
        unsigned short Invulnerable;
      } NameColoring;

      struct
      {
        unsigned short CriminalFlagInterval;
        unsigned short AggressorFlagTimeout;
        bool PartyHelpFullCountsAsCriminal;
      } General;

      struct
      {
        ExportedFunction* NameColor;
        ExportedFunction* HighLightColor;
        ExportedFunction* OnAttack;
        ExportedFunction* OnDamage;
        ExportedFunction* OnHelp;
      } Hooks;

    } repsys_cfg;

    /// [1.1] General Configuration
    ///  Settings are defined in the General section:
    ///	CriminalFlagInterval	 Time, in seconds, for which you will be marked criminal
    ///	AggressorFlagTimeout	 Time, in seconds, for which you will be marked an aggressor
    ///							 
    ///
    /// [1.2] Color Configuration
    ///  Name colors are defined in the NameColoring section:
    ///	Murderer
    ///	Criminal
    ///	Attackable
    ///	Innocent
    ///	GuildAlly
    ///	GuildEnemy
    ///	Invulnerable
    ///  The client specifies highlight colors.
    ///

    void load_repsys_cfg_namecoloring( Clib::ConfigElem& elem )
    {
      repsys_cfg.NameColoring.Murderer = elem.remove_ushort( "Murderer" );
      repsys_cfg.NameColoring.Criminal = elem.remove_ushort( "Criminal" );
      repsys_cfg.NameColoring.Attackable = elem.remove_ushort( "Attackable" );
      repsys_cfg.NameColoring.Innocent = elem.remove_ushort( "Innocent" );
      repsys_cfg.NameColoring.GuildAlly = elem.remove_ushort( "GuildAlly" );
      repsys_cfg.NameColoring.GuildEnemy = elem.remove_ushort( "GuildEnemy" );
      repsys_cfg.NameColoring.Invulnerable = elem.remove_ushort( "Invulnerable" );
    }
    void load_repsys_cfg_general( Clib::ConfigElem& elem )
    {
      repsys_cfg.General.CriminalFlagInterval = elem.remove_ushort( "CriminalFlagInterval" );
      repsys_cfg.General.AggressorFlagTimeout = elem.remove_ushort( "AggressorFlagTimeout" );
      repsys_cfg.General.PartyHelpFullCountsAsCriminal = elem.remove_bool( "PartyHelpFullCountsAsCriminal", false );
    }
    void load_repsys_cfg_hooks( Clib::ConfigElem& elem )
    {
      string temp;
      if ( elem.remove_prop( "NameColor", &temp ) )
        repsys_cfg.Hooks.NameColor = FindExportedFunction( elem, NULL, temp, 2 );
      if ( elem.remove_prop( "HighLightColor", &temp ) )
        repsys_cfg.Hooks.HighLightColor = FindExportedFunction( elem, NULL, temp, 2 );

      if ( elem.remove_prop( "OnAttack", &temp ) )
        repsys_cfg.Hooks.OnAttack = FindExportedFunction( elem, NULL, temp, 2 );
      if ( elem.remove_prop( "OnDamage", &temp ) )
        repsys_cfg.Hooks.OnDamage = FindExportedFunction( elem, NULL, temp, 2 );
      if ( elem.remove_prop( "OnHelp", &temp ) )
        repsys_cfg.Hooks.OnHelp = FindExportedFunction( elem, NULL, temp, 2 );
    }

    void unload_repsys_cfg_hooks()
    {
      if ( repsys_cfg.Hooks.NameColor != NULL )
      {
        delete repsys_cfg.Hooks.NameColor;
        repsys_cfg.Hooks.NameColor = NULL;
      }
      if ( repsys_cfg.Hooks.HighLightColor != NULL )
      {
        delete repsys_cfg.Hooks.HighLightColor;
        repsys_cfg.Hooks.HighLightColor = NULL;
      }
      if ( repsys_cfg.Hooks.OnAttack != NULL )
      {
        delete repsys_cfg.Hooks.OnAttack;
        repsys_cfg.Hooks.OnAttack = NULL;
      }
      if ( repsys_cfg.Hooks.OnDamage != NULL )
      {
        delete repsys_cfg.Hooks.OnDamage;
        repsys_cfg.Hooks.OnDamage = NULL;
      }
      if ( repsys_cfg.Hooks.OnHelp != NULL )
      {
        delete repsys_cfg.Hooks.OnHelp;
        repsys_cfg.Hooks.OnHelp = NULL;
      }
    }

    void unload_repsys_cfg()
    {
      unload_repsys_cfg_hooks();
    }

    void load_repsys_cfg( bool reload )
    {
      Clib::ConfigFile cf( "config/repsys.cfg" );

      Clib::ConfigSection namecoloring( cf, "NameColoring" );
      Clib::ConfigSection general( cf, "General" );
      Clib::ConfigSection hooks( cf, "HookList", Clib::CST_NORMAL );

      Clib::ConfigElem elem;

      while ( cf.read( elem ) )
      {
        if ( namecoloring.matches( elem ) )
        {
          load_repsys_cfg_namecoloring( elem );
        }
        else if ( general.matches( elem ) )
        {
          load_repsys_cfg_general( elem );
        }
        else if ( hooks.matches( elem ) && !reload )
        {
          load_repsys_cfg_hooks( elem );
        }
      }
    }

    ///
    /// [2] Timeouts
    /// When Any of Amy's Rep-system timers time out
    ///
    ///	 If Amy's Criminal Timer has expired,
    ///		 Clear Amy's Criminal Timer
    ///
    ///	 If Amy was Aggressor to Bob, but has timed out
    ///		 De-escalate[->7] Amy and Bob
    ///		 Update Bob to nearby clients
    /// 
    ///	 If Amy had Lawfully Damaged Bob, but has timed out
    ///		 De-escalate[->7] Amy and Bob
    ///		 Update Bob to nearby clients
    ///
    ///	 If any of Amy's statuses changed,
    ///		 Update Amy to nearby clients

    void RepSystem::repsys_task( Mobile::Character* amy )
    {
      THREAD_CHECKPOINT( tasks, 1000 );
      polclock_t now = polclock();
      bool refreshall = false;

      // the 'defensive hostiles' are those that are attacking Amy while she's not innocent.
      // If she's innocent at the end of this, they will break off the attack.
      THREAD_CHECKPOINT( tasks, 1001 );
      Mobile::Character::CharacterSet defensive_hostiles;
      for ( Mobile::Character::CharacterSet::iterator h_itr = amy->opponent_of.begin(); h_itr != amy->opponent_of.end(); ++h_itr )
      {
        Mobile::Character* hostile_bob = *h_itr;
        THREAD_CHECKPOINT( tasks, 1002 );

        if ( !amy->is_innocent_to( hostile_bob ) )
        {
          THREAD_CHECKPOINT( tasks, 1003 );
          defensive_hostiles.insert( hostile_bob );
        }
        THREAD_CHECKPOINT( tasks, 1004 );
      }


      THREAD_CHECKPOINT( tasks, 1005 );
      if ( amy->criminal_until_ && timer_expired( amy->criminal_until_, now ) )
      {
        amy->criminal_until_ = 0;
        refreshall = true;
      }

      THREAD_CHECKPOINT( tasks, 1006 );
      polclock_t earliest_timeout;

      if ( amy->criminal_until_ )
        earliest_timeout = amy->criminal_until_;
      else
        earliest_timeout = now + 5 * 60 * POLCLOCKS_PER_SEC; // 5 minutes, later code will trim

      THREAD_CHECKPOINT( tasks, 1007 );

      Mobile::Character::MobileCont::iterator itr;
      itr = amy->aggressor_to_.begin();
      while ( itr != amy->aggressor_to_.end() )
      {
        THREAD_CHECKPOINT( tasks, 1008 );
        Mobile::Character::MobileCont::iterator save_itr = itr;
        ++itr;
        polclock_t when = ( *save_itr ).second;
        THREAD_CHECKPOINT( tasks, 1009 );
        if ( timer_expired( when, now ) )
        {
          THREAD_CHECKPOINT( tasks, 1010 );
          CharacterRef other = ( *save_itr ).first;
          // not sure if this is right.  what if also lawfully_damaged ?
          THREAD_CHECKPOINT( tasks, 1011 );
          de_escalate( amy, other.get() );
          THREAD_CHECKPOINT( tasks, 1012 );
          amy->aggressor_to_.erase( save_itr );
          THREAD_CHECKPOINT( tasks, 1013 );
          if ( !other->orphan() )
          {
            // Changed to update!
            if ( amy->client )
              send_move( amy->client, amy );
            send_create_mobile_to_nearby_cansee( other.get() );
          }
          else
            Clib::Log( "NOTE: repsys crash avoidance...\n" );
          THREAD_CHECKPOINT( tasks, 1014 );
          refreshall = true;
        }
        else
        {
          earliest_timeout = earliest_timer( when, earliest_timeout );
        }
        THREAD_CHECKPOINT( tasks, 1015 );
      }

      THREAD_CHECKPOINT( tasks, 1016 );
      itr = amy->lawfully_damaged_.begin();
      while ( itr != amy->lawfully_damaged_.end() )
      {
        THREAD_CHECKPOINT( tasks, 1017 );
        Mobile::Character::MobileCont::iterator save_itr = itr;
        ++itr;
        polclock_t when = ( *save_itr ).second;
        THREAD_CHECKPOINT( tasks, 1018 );
        if ( timer_expired( when, now ) )
        {
          THREAD_CHECKPOINT( tasks, 1019 );
          CharacterRef other = ( *save_itr ).first;
          // not sure if this is right.  what if also aggressor_to ?
          THREAD_CHECKPOINT( tasks, 1020 );
          de_escalate( amy, other.get() );
          THREAD_CHECKPOINT( tasks, 1021 );
          amy->lawfully_damaged_.erase( save_itr );
          THREAD_CHECKPOINT( tasks, 1022 );
          if ( !other->orphan() )
          {
            // Changed to update!
            if ( amy->client )
              send_move( amy->client, amy );
            send_create_mobile_to_nearby_cansee( other.get() );
          }
          else
            Clib::Log( "NOTE: repsys crash avoidance...\n" );
          THREAD_CHECKPOINT( tasks, 1023 );
          refreshall = true;
        }
        else
        {
          earliest_timeout = earliest_timer( when, earliest_timeout );
        }
        THREAD_CHECKPOINT( tasks, 1024 );
      }

      THREAD_CHECKPOINT( tasks, 1025 );
      // For those that were hostile to Amy due to her non-Innocence, if she's Innocent now,
      // break off the attack.
      for ( Mobile::Character::CharacterSet::iterator h_itr = defensive_hostiles.begin(); h_itr != defensive_hostiles.end(); ++h_itr )
      {
        Mobile::Character* bob = *h_itr;
        THREAD_CHECKPOINT( tasks, 1026 );

        if ( amy->is_innocent_to( bob ) )
        {
          THREAD_CHECKPOINT( tasks, 1027 );
          de_escalate( amy, bob );
          THREAD_CHECKPOINT( tasks, 1028 );

          refreshall = true;
        }
      }

      THREAD_CHECKPOINT( tasks, 1029 );

      if ( amy->criminal_until_ ||
           !amy->aggressor_to_.empty() ||
           !amy->lawfully_damaged_.empty() )
      {
        THREAD_CHECKPOINT( tasks, 1030 );
        schedule_repsys_task( amy, earliest_timeout + 1 );
      }

      THREAD_CHECKPOINT( tasks, 1031 );
      if ( refreshall )
      {
        THREAD_CHECKPOINT( tasks, 1032 );
        if ( !amy->orphan() )
        {
          // Changed to update!
          if ( amy->client )
            send_move( amy->client, amy );
          send_create_mobile_to_nearby_cansee( amy );
        }
      }
      THREAD_CHECKPOINT( tasks, 1099 );
    }



    void RepSystem::schedule_repsys_task( Mobile::Character* chr, polclock_t runat )
    {
      if ( chr->repsys_task_ != NULL )
      {
        if ( chr->repsys_task_->next_run_clock() > runat )
          chr->repsys_task_->cancel();
      }

      if ( chr->repsys_task_ == NULL )
      {
        new OneShotTaskInst<Mobile::Character*>( &chr->repsys_task_,
                                                 runat,
                                                 RepSystem::repsys_task,
                                                 chr );
      }
    }
    bool private_say_above_ex( Mobile::Character* chr,
                               const UObject* obj,
                               const char *text,
                               unsigned short color );
    // bool RepSystem::can_attack( Character* attacker, Character* defender )
    // {
    //	return (defender->is_aggressor_to( attacker ) || 
    //			attacker->has_lawfully_damaged(defender) ||
    //			defender->is_criminal());
    // }
    //

    // a scenario:
    // A (innocent) attacks B (innocent):
    //   (3), so A criminal, aggressor to B
    // A (crim,agg) attacks B (innocent):
    //   (1), so A's aggressor timer restarts (not criminal timer)
    // B (innocent) attacks A (crim,agg):
    //   (2b), so lawfully damaged timer restarts
    // B (innocent) attacks A (crim,agg):
    //   (2a), so lawfully damaged timer restarts
    // A (crim, agg) attacks B (innocent):
    //   umm, what should happen?


    ///
    /// [3] Player attacks another Player
    /// When Amy attacks Bob
    ///	 These rules are applied:
    ///		 - When Amy applies damage to Bob (on_pc_damaged_pc calls this)
    ///		 - repsys_on_attack (Amy attacks PC, or tamed creature)
    ///			  - apply_raw_damage_hundredths
    ///			  - amy sets opponent
    ///			  - melee attack
    ///			  - amy's NPC does any of these things
    ///		 - When Amy tries to apply any damage to Bob
    ///		 - When Amy selects Bob as her Opponent
    ///		 - When Amy swings at Bob
    ///		 - When Amy selects Bob with a Harmful target cursor
    ///
    ///   if Bob is Innocent to Amy,
    ///	   Set Amy's Criminal flag for CriminalFlagInterval seconds
    ///
    ///   if Bob is Aggressor to Amy,
    ///	   Set Bob as Aggressor to Amy for AggressorFlagTimeout seconds
    ///   else,
    ///	   Tell Bob "Amy is Attacking you!" if this is news
    ///	   Set Amy as Aggressor to Bob for AggressorFlagTimeout seconds
    ///

    void RepSystem::on_pc_attacks_pc( Mobile::Character* amy_attacker, Mobile::Character* bob_defender )
    {
      if ( amy_attacker == bob_defender )
        return;

      bool refresh = false;

      polclock_t now = polclock();
      polclock_t crim_timeout_at = now + repsys_cfg.General.CriminalFlagInterval * POLCLOCKS_PER_SEC;
      polclock_t aggr_timeout_at = now + repsys_cfg.General.AggressorFlagTimeout * POLCLOCKS_PER_SEC;

      if ( bob_defender->is_innocent_to( amy_attacker ) )
      {
        if ( !amy_attacker->is_criminal() )
          refresh = true;
        amy_attacker->restart_criminal_timer( crim_timeout_at );
      }

      if ( bob_defender->is_aggressor_to( amy_attacker ) )
      {
        bob_defender->restart_aggressor_timer( amy_attacker, aggr_timeout_at );
      }
      else // he's not the aggressor, so I guess I am.
      {
        if ( !amy_attacker->is_aggressor_to( bob_defender ) )
        {
          if ( combat_config.send_attack_msg )
          {
            string msg = "*" + amy_attacker->name() + " is attacking you!*";
            private_say_above_ex( bob_defender, bob_defender, msg.c_str(), repsys_cfg.NameColoring.Murderer );
          }
          refresh = true;
        }
        amy_attacker->restart_aggressor_timer( bob_defender, aggr_timeout_at );
      }

      if ( refresh )
      {
        // Changed to update!
        if ( amy_attacker->client )
          send_move( amy_attacker->client, amy_attacker );
        send_create_mobile_to_nearby_cansee( amy_attacker );
      }
      polclock_t timeout_at = earliest_timer( crim_timeout_at, aggr_timeout_at );

      schedule_repsys_task( amy_attacker, timeout_at + 1 );
      schedule_repsys_task( bob_defender, timeout_at + 1 );
    }

    ///
    /// [4] Player Damages Player
    /// When Amy damages Bob
    ///	 These rules applied:
    ///		 - When Amy actually applies damage to Bob
    ///		 - When Amy poisons Bob (only with Bob.SetPoisoned(1))
    ///			(note damage due to poisoning will not fire this rule,
    ///			 only the initial application)
    ///		 - When Amy paralyzes Bob (only with Bob.SetParalyzed(1))
    ///
    ///	 Apply all rules for "Player Attacks Another Player"[->3]
    ///
    ///	 If Bob is Innocent to Amy,
    ///		 Add Amy to Bob's "ToBeReportable" list
    ///	 else
    ///		 Set Amy as having LawfullyDamaged Bob for AggressorFlagTimeout seconds
    ///
    ///	 
    /// 
    void RepSystem::on_pc_damages_pc( Mobile::Character* amy, Mobile::Character* bob )
    {
      if ( amy == bob )
        return;
      polclock_t aggr_timeout_at = polclock() + repsys_cfg.General.AggressorFlagTimeout * POLCLOCKS_PER_SEC;

      on_pc_attacks_pc( amy, bob );

      if ( bob->is_innocent_to( amy ) )
      {
        bob->add_to_be_reportable( amy->serial );
      }
      else
      {
        amy->restart_lawfully_damaged_timer( bob, aggr_timeout_at );
        schedule_repsys_task( amy, aggr_timeout_at + 1 );
      }
    }



    ///
    /// [5] Player Helps Another Player
    /// When Amy Helps Bob
    ///	 These rules applied:
    ///		 - When Amy selects Bob with a Helpful target cursor
    ///		 - When Amy Heals Bob
    ///		 - When Amy clear's Bob's Poisoned flag
    ///		 - When Amy clear's Bob's Paralyzed flag
    ///
    /// If PartyHelpFullCountsAsCriminal false and same party dont set
    ///	If Bob is a Criminal, Set Amy Criminal 
    ///	

    void RepSystem::on_pc_helps_pc( Mobile::Character* amy, Mobile::Character* bob )
    {
      if ( ( amy != bob ) && bob->is_criminal() )
      {
        if ( !repsys_cfg.General.PartyHelpFullCountsAsCriminal )
        {
          if ( ( amy->party() != NULL ) && ( amy->party()->is_member( bob->serial ) ) )
            return;
        }
        amy->make_criminal();
      }
    }

    /// [7] De-Escalation
    /// To De-escalate Amy and Bob:
    ///	  if Amy's opponent is Bob, Amy clears her opponent
    ///	  If Bob's opponent is Amy, Bob clears his opponent   
    ///

    void RepSystem::de_escalate( Mobile::Character* amy, Mobile::Character* bob )
    {
      if ( amy->opponent_ == bob )
      {
        amy->set_opponent( NULL, true );
      }

      if ( bob->opponent_ == amy )
      {
        bob->set_opponent( NULL, true );
      }
    }

    ///
    /// [8] Highlighting and Name Coloring
    ///	if Amy looks at Bob, coloring is as follows:
    ///		if Bob is invul and invul tag set 2, color Bob INVUL (yellow)
    ///		if Bob is a Murderer			  color Bob MURDERER ()
    ///		if Bob is a Criminal,			 color Bob CRIMINAL (Red)
    ///		If Bob is a guild ally of Amy's,  color Bob FRIEND (Green)
    ///		if Bob is an Aggressor to Amy,	color Bob ATTACKABLE (Grey)
    ///		if Amy has lawfully damaged Bob,  color Bob ATTACKABLE (Grey)
    ///		if Bob is a guild enemy of Amy's, color Bob ENEMY (Orange)
    ///		otherwise						 color Bob INNOCENT (Blue)
    ///

    unsigned char RepSystem::hilite_color_idx( const Mobile::Character* amy, const Mobile::Character* bob )
    {

      if ( repsys_cfg.Hooks.HighLightColor )
      {
        Mobile::Character* t_amy = const_cast<Mobile::Character*>( amy );
        Mobile::Character* t_bob = const_cast<Mobile::Character*>( bob );
        return (unsigned char)( repsys_cfg.Hooks.HighLightColor->call_long( t_bob->make_ref(), t_amy->make_ref() ) );
      }

      if ( ( ssopt.invul_tag == 2 ) && bob->invul() )
        return CHAR_HILITE_INVUL;
      if ( bob->is_murderer() )
        return CHAR_HILITE_MURDERER;
      if ( bob->is_criminal() )
        return CHAR_HILITE_CRIMINAL;
      if ( bob->is_guild_ally( amy ) )
        return CHAR_HILITE_FRIEND;
      if ( bob->is_aggressor_to( amy ) || amy->has_lawfully_damaged( bob ) )
        return CHAR_HILITE_ATTACKABLE;
      if ( bob->is_guild_enemy( amy ) )
        return CHAR_HILITE_ENEMY;

      return CHAR_HILITE_INNOCENT;
    }

    /*
     * AMY = Mobile doing the seeing
     * BOB = Mobile being seen.
     *
     */
    unsigned short RepSystem::name_color( const Mobile::Character* amy, const Mobile::Character* bob )
    {
      if ( repsys_cfg.Hooks.NameColor )
      {
        Mobile::Character* t_amy = const_cast<Mobile::Character*>( amy );
        Mobile::Character* t_bob = const_cast<Mobile::Character*>( bob );
        return (unsigned short)( repsys_cfg.Hooks.NameColor->call_long( t_bob->make_ref(), t_amy->make_ref() ) );
      }

      if ( ssopt.invul_tag == 2 )
      {
        if ( bob->invul() )
          return repsys_cfg.NameColoring.Invulnerable;
      }
      if ( bob->is_murderer() )
        return repsys_cfg.NameColoring.Murderer;
      if ( bob->is_criminal() )
        return repsys_cfg.NameColoring.Criminal;
      else if ( bob->is_guild_ally( amy ) )
        return repsys_cfg.NameColoring.GuildAlly;
      else if ( bob->is_aggressor_to( amy ) || amy->has_lawfully_damaged( bob ) )
        return repsys_cfg.NameColoring.Attackable;
      else if ( bob->is_guild_enemy( amy ) )
        return repsys_cfg.NameColoring.GuildEnemy;
      else
        return repsys_cfg.NameColoring.Innocent;
    }
  }
  namespace Mobile {

    ///
    /// [9] Innocent Status
    /// Bob is Innocent to Amy if:
    /// ===============================
    ///	Bob is Innocent to Amy only if NONE of the following are true:
    ///		 Bob is a murderer;
    ///		 Bob is a criminal
    ///		 Bob is an Aggressor to Amy
    ///		 Bob is a Guild Ally of Amy
    ///		 Bob is a Guild Enemy of Amy
    ///		 Amy has Lawfully Damaged Bob
    ///
    bool Character::is_innocent_to( const Character* amy ) const
    {
      // this == bob
      const Character& bob = *this;

      if ( bob.is_murderer() )				  return false;

      if ( bob.is_criminal() )				  return false;

      if ( bob.is_aggressor_to( amy ) )		 return false;

      if ( bob.is_guild_ally( amy ) )		   return false;

      if ( bob.is_guild_enemy( amy ) )		  return false;

      if ( amy->has_lawfully_damaged( &bob ) )  return false;

      return true;
    }

    bool Character::is_aggressor_to( const Character* chr ) const
    {
      Character* in_chr = const_cast<Character*>( chr );
      return ( aggressor_to_.find( Core::CharacterRef( in_chr ) ) != aggressor_to_.end() );
    }

    ///
    /// [10] Guild Rules
    /// The following Guild Rules are only temporary pending a more complete system
    ///
    /// [10.1] Guild Allies
    /// Bob and Amy are Guild Allies if:
    ///	 Bob is in a guild, AND
    ///	 Amy is in a guild, AND
    ///	 Bob's Guild is allied with Amy's guild
    ///
    bool Character::is_guild_ally( const Character* chr ) const
    {
      return ( guildid() &&
               chr->guildid() &&
               AreAllies( guild(), chr->guild() ) );
    }

    ///
    /// [10.2] Guild Enemies
    /// Bob and Amy are Guild Enemies if:
    ///	 Bob is in a guild, AND
    ///	 Amy is in a guild, AND
    ///	 Bob's Guild is an Enemy Guild of Amy's Guild
    ///
    ///
    bool Character::is_guild_enemy( const Character* chr ) const
    {
      return ( guildid() &&
               chr->guildid() &&
               AreEnemies( guild(), chr->guild() ) );
    }

    //
    // To Mark Bob an Aggressor to Amy:
    //	 Bob is reported as an Aggressor to Amy for 2 minutes
    //	
    //
    void Character::restart_aggressor_timer( Character* amy, Core::polclock_t until )
    {
      MobileCont::iterator itr = aggressor_to_.find( Core::CharacterRef( amy ) );
      if ( itr == aggressor_to_.end() )
      {
        aggressor_to_[Core::CharacterRef( amy )] = until;
      }
      else
      {
        if ( until > ( *itr ).second )
          ( *itr ).second = until;
      }
    }

    bool Character::has_lawfully_damaged( const Character* chr ) const
    {
      Character* in_chr = const_cast<Character*>( chr );
      return ( lawfully_damaged_.find( Core::CharacterRef( in_chr ) ) != lawfully_damaged_.end( ) );
    }

    void Character::restart_lawfully_damaged_timer( Mobile::Character* amy, Core::polclock_t when )
    {
      MobileCont::iterator itr = lawfully_damaged_.find( Core::CharacterRef( amy ) );
      if ( itr == lawfully_damaged_.end() )
      {
        lawfully_damaged_[Core::CharacterRef( amy )] = when;
      }
      else
      {
        if ( when > ( *itr ).second )
          ( *itr ).second = when;
      }
    }

    ///
    /// A Mobile is Criminal if:
    ///	 he has an active Criminal Timer, which has not timed out.
    ///	   OR
    ///	 he is a murderer.
    ///
    bool Character::is_criminal() const
    {
      return ( Core::polclock( ) <= criminal_until_ ) || murderer_;
    }

    void Character::restart_criminal_timer( Core::polclock_t until )
    {
      if ( until > criminal_until_ )
      {
        if ( criminal_until_ < Core::polclock( ) )
        {
          ForEachMobileInRange( static_cast<unsigned short>( x ), static_cast<unsigned short>( y ), realm, 32,
                                NpcPropagateCriminal, this );
        }
        criminal_until_ = until;
      }
    }

    void Character::clear_criminal_timer()
    {
      criminal_until_ = 0;
    }

    bool Character::is_murderer() const
    {
      return murderer_;
    }


    //
    // Everything below this point is DOUBLE-DISPATCH for 
    //  PC vs NPC, NPC vs PC, NPC vs NPC
    //  as well as PC -> NPC reactions.
    //

    /* the reputation system only comes into play if:
          a player attacks a player
          a player attacks a player's creature
          a player's creature attacks a player
          a player's creature attacks a player's creature
          */




    ///
    /// [13] Mobile (MA) Attacks Mobile (MA)
    ///
    ///	 Rules are applied based on whether MA is a PC or an NPC.
    ///  
    /// [13.1] Player (Amy) Attacks Mobile (Mob)
    ///	 If Mob is a PC (Bob),
    ///		 Apply 'Player Attacks Player' rules[->3] for Amy vs Bob.
    ///	 Else if Mob is an NPC with a Master (Bob),
    ///		 Apply 'Player Attacks Player' rules for Amy vs Bob
    ///	 Else if Mob is a Good-aligned NPC,
    ///		 Set Amy's Criminal Flag
    ///
    void Character::repsys_on_attack( Character* defender )
    {
      if ( Core::repsys_cfg.Hooks.OnAttack )
      {
        if ( Core::repsys_cfg.Hooks.OnAttack->call( this->make_ref( ), defender->make_ref( ) ) )
          return;
      }

      // consider double-dispatch here.
      if ( defender->isa( CLASS_CHARACTER ) )
      {
        Core::RepSystem::on_pc_attacks_pc( this, defender );
      }
      else
      {
        Core::NPC* npc = static_cast<Core::NPC*>( defender );
        if ( npc->master() )
        {
          Core::RepSystem::on_pc_attacks_pc( this, npc->master( ) );
        }
        else if ( ( npc->alignment( ) == Core::NpcTemplate::GOOD ) && ( !npc->is_criminal( ) ) )
        {
          make_criminal();
        }
      }
    }
  }
  namespace Core {

    ///
    /// [13.2] NPC (MA) Attacks Mobile (MB)
    ///	 If MA has a Master (Amy),
    ///		 Apply 'Player Attacks Mobile' rules[->13.1] for Amy vs MB
    ///	 Else
    ///		 Exit with no (RepSystem) effect.
    ///
    void NPC::repsys_on_attack( Character* defender )
    {
      if ( repsys_cfg.Hooks.OnAttack )
      {
        if ( repsys_cfg.Hooks.OnAttack->call( this->make_ref(), defender->make_ref() ) )
          return;
      }

      if ( master() )
      {
        master_->Character::repsys_on_attack( defender );
      }
    }
  }

  namespace Mobile {


    ///
    /// [14] Mobile (MA) Damages Mobile (MB)
    ///
    ///	 Rules are applied based on whether MA is a PC or an NPC.
    ///
    /// [14.1] Player (Amy) Damages Mobile (Mob)
    ///   If Mob is a PC (Bob),
    ///	   Apply 'Player Damages Player' rules[->4] for Amy vs Bob.
    ///   Else if Mob is an NPC with a Master (Bob),
    ///	   Apply 'Player Damages Player' rules for Amy vs Bob
    ///   Else if Mob is a Good-aligned NPC,
    ///	   Set Amy's Criminal Flag
    ///
    void Character::repsys_on_damage( Character* defender )
    {
      if ( Core::repsys_cfg.Hooks.OnDamage )
      {
        if ( Core::repsys_cfg.Hooks.OnDamage->call( this->make_ref(), defender->make_ref() ) )
          return;
      }

      if ( defender->isa( CLASS_CHARACTER ) )
      {
        Core::RepSystem::on_pc_damages_pc( this, defender );
      }
      else
      {
        Core::NPC* npc = static_cast<Core::NPC*>( defender );
        if ( npc->master() )
        {
          Core::RepSystem::on_pc_damages_pc( this, npc->master( ) );
        }
        else if ( ( npc->alignment( ) == Core::NpcTemplate::GOOD ) && ( !npc->is_criminal( ) ) )
        {
          make_criminal();
        }
      }
    }
  }
  namespace Core {

    ///
    /// [14.2] NPC (MA) Damages Mobile (MB)
    ///	 If MA has a Master (Amy),
    ///		 Apply 'Player Damages Mobile' rules[->14.1] for Amy vs MB
    ///	 Else
    ///		 Exit with no (RepSystem) effect.
    ///
    void NPC::repsys_on_damage( Character* defender )
    {
      if ( repsys_cfg.Hooks.OnDamage )
      {
        if ( repsys_cfg.Hooks.OnDamage->call( this->make_ref(), defender->make_ref() ) )
          return;
      }

      if ( master() )
        master_->Character::repsys_on_damage( defender );
    }
  }
  namespace Mobile {

    ///
    /// [15] Mobile (MA) Helps Mobile (MB)
    ///
    ///	 Rules are applied based on whether MA is a PC or an NPC.
    /// 
    ///   [15.1] Player (Amy) Helps Mobile (Mob)
    ///	 If Mob is a PC (Bob),
    ///		 Apply 'Player Helps Player' rules[->5] for Amy helps Bob.
    ///	 Else if Mob is an NPC with a Master (Bob),
    ///		 Apply 'Player Helps Player' rules for Amy helps Bob
    ///	 Else if Mob is an Evil-aligned NPC,
    ///		 Set Amy's Criminal Flag

    void Character::repsys_on_help( Character* helped )
    {
      if ( Core::repsys_cfg.Hooks.OnHelp )
      {
        if ( Core::repsys_cfg.Hooks.OnHelp->call( this->make_ref(), helped->make_ref() ) )
          return;
      }

      // repsys_on_pc_helps( helped );
      if ( helped->isa( CLASS_CHARACTER ) )
      {
        Core::RepSystem::on_pc_helps_pc( this, helped );
      }
      else
      {
        Core::NPC* npc = static_cast<Core::NPC*>( helped );
        if ( npc->master() )
        {
          Core::RepSystem::on_pc_helps_pc( this, npc->master( ) );
        }
        else if ( npc->alignment( ) == Core::NpcTemplate::EVIL )
        {
          make_criminal();
        }
      }
    }
  }
  namespace Core {
    ///
    ///   [15.2] NPC (MA) Helps Mobile (MB)
    ///	   If MA has a Master (Amy),
    ///		   Apply 'Player Helps Mobile' rules[->15.1] for Amy helps MB
    ///	   Else
    ///		   Exit with no (RepSystem) effect.
    ///
    void NPC::repsys_on_help( Character* helped )
    {
      if ( repsys_cfg.Hooks.OnHelp )
      {
        if ( repsys_cfg.Hooks.OnHelp->call( this->make_ref(), helped->make_ref() ) )
          return;
      }

      // repsys_on_npc_helps( helped );
      if ( master() )
        master_->Character::repsys_on_help( helped );
    }

  }
  namespace Mobile {

    unsigned char Character::hilite_color_idx( const Character* seen_by ) const
    {
      return Core::RepSystem::hilite_color_idx( seen_by, this );
    }
    unsigned short Character::name_color( const Character* seen_by ) const
    {
      return Core::RepSystem::name_color( seen_by, this );
    }
  }
  namespace Core {


    ///
    /// [16] NPC Highlighting
    ///	 If the NPC has a master, 
    ///		 Highlight color is the same as the Master's.
    ///	 Else the NPC is invul
    ///		 Hilite them invul is ssopt.cfg option says to.
    ///	 Else
    ///		 Highlight Good NPCs	 INNOCENT (Blue)
    ///		 Highlight Neutral NPCs  ATTACKABLE (Grey)
    ///		 Highlight Evil NPCs	 MURDERER (Red)
    ///
    unsigned char NPC::hilite_color_idx( const Character* seen_by ) const
    {
      if ( repsys_cfg.Hooks.HighLightColor )
      {
        NPC* t_amy = const_cast<NPC*>( this );
        Character* t_bob = const_cast<Character*>( seen_by );
        int tmp = repsys_cfg.Hooks.HighLightColor->call_long( t_amy->make_ref(), t_bob->make_ref() );
        return (unsigned char)( tmp );
      }

      if ( ssopt.invul_tag == 2 )
      {
        if ( invul() )
          return CHAR_HILITE_INVUL;
      }
      if ( master() )
      {
        return Core::RepSystem::hilite_color_idx( seen_by, master( ) );
      }
      else
      {
        switch ( template_.alignment )
        {
          case NpcTemplate::NEUTRAL:
            return CHAR_HILITE_ATTACKABLE;
          case NpcTemplate::GOOD:
            if ( is_murderer() )
              return CHAR_HILITE_MURDERER;
            else if ( is_criminal() )
              return CHAR_HILITE_ATTACKABLE;
            else
              return CHAR_HILITE_INNOCENT;
          case NpcTemplate::EVIL:
            return CHAR_HILITE_MURDERER;
        }
        return CHAR_HILITE_ATTACKABLE;
      }
    }

    unsigned short NPC::name_color( const Character* seen_by ) const
    {
      if ( repsys_cfg.Hooks.NameColor )
      {
        NPC* t_amy = const_cast<NPC*>( this );
        Character* t_bob = const_cast<Character*>( seen_by );
        return (unsigned short)( repsys_cfg.Hooks.NameColor->call_long( t_amy->make_ref(), t_bob->make_ref() ) );
      }

      if ( ssopt.invul_tag == 2 )
      {
        if ( this->invul() )
          return repsys_cfg.NameColoring.Invulnerable;
      }
      if ( master() )
      {
        return RepSystem::name_color( seen_by, master( ) );
      }
      else
      {
        switch ( template_.alignment )
        {
          case NpcTemplate::NEUTRAL:
            return repsys_cfg.NameColoring.Attackable;
          case NpcTemplate::GOOD:
            if ( is_murderer() )
              return repsys_cfg.NameColoring.Murderer;
            else if ( is_criminal() )
              return repsys_cfg.NameColoring.Criminal;
            else
              return repsys_cfg.NameColoring.Innocent;
          case NpcTemplate::EVIL:
            return repsys_cfg.NameColoring.Murderer;
        }

        return repsys_cfg.NameColoring.Attackable;
      }
    }
  }
  namespace Mobile {
	///
	/// To Set Amy Criminal (For a LevelOfOffense)
	///
	///	If the LevelOfOffense is 0,
	///		 clear the CriminalTimer
	///	Else
	///		 Set her Criminal Timer for (LevelOfOffense * CriminalFlagInterval) seconds
	///

	void Character::make_criminal( int level )
	{
	  set_dirty();
	  bool was_criminal = is_criminal();

	  if ( level )
	  {
        Core::polclock_t timeout_at = Core::polclock( ) + level * Core::repsys_cfg.General.CriminalFlagInterval * Core::POLCLOCKS_PER_SEC;

		restart_criminal_timer( timeout_at );
        Core::RepSystem::schedule_repsys_task( this, timeout_at + 1 );
	  }
	  else
	  {
		clear_criminal_timer();
        Core::RepSystem::schedule_repsys_task( this, Core::polclock( ) - 1 );
	  }

	  if ( !orphan() && was_criminal != is_criminal() )
	  {
		// Changed to update!
		on_criminal_changed();
	  }
	}

	void Character::make_murderer( bool newval )
	{
	  bool refresh = ( murderer_ != newval );

	  set_dirty();
	  murderer_ = newval;

	  if ( !orphan() && refresh )
	  {
		// Changed to update!
		on_murderer_changed();
	  }
      Core::RepSystem::schedule_repsys_task( this, Core::polclock( ) - 1 );
	}

	void Character::make_aggressor_to( Character* chr )
	{
      Core::polclock_t aggr_timeout_at = Core::polclock( ) + Core::repsys_cfg.General.AggressorFlagTimeout * Core::POLCLOCKS_PER_SEC;
	  restart_aggressor_timer( chr, aggr_timeout_at );

	  // Changed to update!
	  on_aggressor_changed();

      Core::RepSystem::schedule_repsys_task( this, aggr_timeout_at + 1 );
	}

	void Character::make_lawfullydamaged_to( Character* chr )
	{
      Core::polclock_t aggr_timeout_at = Core::polclock( ) + Core::repsys_cfg.General.AggressorFlagTimeout * Core::POLCLOCKS_PER_SEC;
	  restart_lawfully_damaged_timer( chr, aggr_timeout_at );

	  // Changed to update!
	  on_lawfullydamaged_changed();

      Core::RepSystem::schedule_repsys_task( this, aggr_timeout_at + 1 );
	}

	// add serial to Bob's ToBeReportable list
	void Character::add_to_be_reportable( u32 serial )
	{
	  set_dirty();
	  to_be_reportable_.insert( serial );
	}
	void Character::clear_to_be_reportables()
	{
	  set_dirty();
	  to_be_reportable_.clear();
	}
	void Character::commit_to_reportables()
	{
	  set_dirty();
      Core::polclock_t now = Core::polclock( );

	  ToBeReportableList::iterator itr = to_be_reportable_.begin(),
		end = to_be_reportable_.end();
	  for ( ; itr != end; ++itr )
	  {
		reportable_t rt;
		rt.serial = ( *itr );
		rt.polclock = now;
		reportable_.insert( rt );
	  }

	  to_be_reportable_.clear();
	}
    void Character::clear_reportable( u32 serial, Core::polclock_t when )
	{
	  set_dirty();
	  reportable_t rt;
	  rt.serial = serial;
	  rt.polclock = when;
	  reportable_.erase( rt );
	}
	void Character::clear_my_aggressors()
	{
	  // we don't actually have a list of aggressors.
	  ToBeReportableList::iterator itr = to_be_reportable_.begin(),
		end = to_be_reportable_.end();
	  for ( ; itr != end; ++itr )
	  {
		u32 serial = ( *itr );
        Character* aggressor = Core::system_find_mobile( serial );
		if ( aggressor != NULL )
		{
		  aggressor->remove_as_aggressor_to( this );
		}
	  }
	}
	void Character::remove_as_aggressor_to( Character* chr )
	{
      Core::CharacterRef ref( chr );
	  aggressor_to_.erase( ref );
	  if ( chr->client != NULL )
	  {
		send_create_mobile_if_nearby_cansee( chr->client, this );
	  }
	  if ( client != NULL )
	  {
		send_create_mobile_if_nearby_cansee( client, chr );
	  }
	}
	void Character::clear_my_lawful_damagers()
	{
	  // we don't actually have a list of aggressors.
	  ToBeReportableList::iterator itr = to_be_reportable_.begin(),
		end = to_be_reportable_.end();
	  for ( ; itr != end; ++itr )
	  {
		u32 serial = ( *itr );
        Character* damager = Core::system_find_mobile( serial );
		if ( damager != NULL )
		{
		  damager->remove_as_lawful_damager( this );
		}
	  }
	}

	void Character::remove_as_lawful_damager( Character* chr )
	{
      Core::CharacterRef ref( chr );
	  lawfully_damaged_.erase( ref );

	  if ( chr->client != NULL )
	  {
		send_create_mobile_if_nearby_cansee( chr->client, this );
	  }
	  if ( client != NULL )
	  {
		send_create_mobile_if_nearby_cansee( client, chr );
	  }
	}

	// Keep these as separate functions if there will be in the future something specific for them
	// For now they are just copies of each other

	void Character::on_criminal_changed()
	{
	  if ( this->client )
		send_move( this->client, this );
	  send_create_mobile_to_nearby_cansee( this );
	}

	void Character::on_murderer_changed()
	{
	  if ( this->client )
		send_move( this->client, this );
	  send_create_mobile_to_nearby_cansee( this );
	}

	void Character::on_aggressor_changed()
	{
	  if ( this->client )
		send_move( this->client, this );
	  send_create_mobile_to_nearby_cansee( this );
	}

	void Character::on_lawfullydamaged_changed()
	{
	  if ( this->client )
		send_move( this->client, this );
	  send_create_mobile_to_nearby_cansee( this );
	}

  }
}