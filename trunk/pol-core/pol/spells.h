/*
History
=======


Notes
=======

*/

#ifndef SPELLS_H
#define SPELLS_H

#include <string>
#include <vector>
#include <time.h>

#include "../clib/refptr.h"

#include "action.h"
#include "scrdef.h"
#include "schedule.h"
#include "reftypes.h"

namespace Pol {
  namespace Clib {
	class ConfigElem;
  }
  namespace Plib {
	class Package;
  }
  namespace Mobile {
	class Character;
  }
  namespace Network {
	class Client;
  }
  namespace Core {
	class TargetCursor;
	class USpell;

	extern u32 spell_scroll_objtype_limits[8][2];

	class USpellParams
	{
	public:
	  USpellParams();
	  USpellParams( Clib::ConfigElem& elem );
	  // default ok USpellParams( const USpellParams& );

	public:
	  unsigned short manacost;
	  unsigned short difficulty;
	  unsigned short pointvalue;
	  unsigned short delay;

	  // unsigned short magery_req_;
	  //unsigned short magery_auto_success_; 
	};


	class USpell
	{
	public:
	  USpell( Clib::ConfigElem& elem, Plib::Package* pkg );

	  const unsigned short spell_id() const;
	  const std::string& name() const;

	  void cast( Mobile::Character* caster );
	  unsigned short manacost() const;
	  unsigned short delay() const;
	  unsigned short difficulty() const;

	  bool consume_reagents( Mobile::Character* chr );
	  bool check_mana( Mobile::Character* chr );
	  bool check_skill( Mobile::Character* chr );
	  void consume_mana( Mobile::Character* chr ); // assumes check_mana has returned true
	  void speak_power_words( Mobile::Character* chr, unsigned short font, unsigned short color );
	  UACTION animation() const;

	  friend void register_spell( USpell *spell, unsigned short spellid );

	protected:

	  Plib::Package* pkg_;

	  unsigned short spellid_;
	  std::string name_;
	  std::string power_words_;
	  UACTION action_;

	  typedef std::vector<unsigned int> RegList;
	  RegList reglist_;
	  USpellParams params_;

	  /* Only one of these will be set. */
	  ScriptDef scriptdef_;
	  /*
		  string script_;
		  void (*f_)(Character* caster);
		  TargetCursor *tcursor_;
		  */

	  static void register_spell( USpell *spell );
	};
	inline unsigned short USpell::manacost() const
	{
	  return params_.manacost;
	}
	inline unsigned short USpell::delay() const
	{
	  return params_.delay;
	}
	inline unsigned short USpell::difficulty() const
	{
	  return params_.difficulty;
	}
	inline const std::string& USpell::name() const
	{
	  return name_;
	}
	inline UACTION USpell::animation() const
	{
	  return action_;
	}
	inline const unsigned short USpell::spell_id() const
	{
	  return spellid_;
	}

	//extern USpell *spells[ SPELLID__COUNT+1 ];

	extern std::vector<USpell*> spells2;
	inline bool VALID_SPELL_ID( int spellid )
	{
	  return ( spellid >= 1 && spellid <= int( spells2.size() ) );
	}

	void do_cast( Network::Client *client, u16 spellid );
	void clean_spells();

	class SpellTask : public OneShotTask
	{
	public:
	  SpellTask( OneShotTask** handle, polclock_t run_when, Mobile::Character* caster, USpell* spell, bool dummy );
	  virtual void on_run();

	private:
	  CharacterRef caster_;
	  USpell* spell_;
	};

  }
}
#endif
