/*
History
=======
2006/09/17 Shinigami: send_event() will return error "Event queue is full, discarding event"
2009/09/22 MuadDib:   Rewrite for Character/NPC to use ar(), ar_mod(), ar_mod(newvalue) virtuals.
2009/11/16 Turley:    added NpcPropagateEnteredArea()/inform_enteredarea() for event on resurrection
2010/02/03 Turley:    MethodScript support for mobiles

Notes
=======

*/

#ifndef H_NPC_H
#define H_NPC_H

#ifndef __CHARACTR_H
#	include "mobile/charactr.h"
#endif

#ifdef _MSC_VER
#	pragma warning(disable:4786)
#endif

#include <iosfwd>
#include <map>
#include <string>


#include "eventid.h"
#include "npctmpl.h"
#include "uoscrobj.h"

namespace Pol {
  namespace Clib {
	class ConfigElem;
  }
  namespace Module {
    class NPCExecutorModule;
  }
  namespace Core {
	class UOExecutor;
	
	class RepSystem;

	class NPC : public Mobile::Character
	{
	  typedef Mobile::Character base;
	public:
	  explicit NPC( u32 objtype, const Clib::ConfigElem& elem );
      virtual ~NPC();
      virtual size_t estimatedSize() const;

	  virtual void on_death( Items::Item* corpse );
	  virtual void on_give_item();

	  // Speech: ASCII versions
	  virtual void on_pc_spoke( Mobile::Character *src_chr, const char *speech, u8 texttype );
	  virtual void on_ghost_pc_spoke( Mobile::Character* src_chr, const char* speech, u8 texttype );

	  // Speech: Unicode (and ASCII) versions
	  virtual void on_pc_spoke( Mobile::Character *src_chr, const char *speech, u8 texttype,
								const u16* wspeech, const char lang[4], Bscript::ObjArray* speechtokens = NULL );
	  virtual void on_ghost_pc_spoke( Mobile::Character* src_chr, const char* speech, u8 texttype,
                                      const u16* wspeech, const char lang[4], Bscript::ObjArray* speechtokens = NULL );

	  virtual unsigned short ar() const;
	  virtual void refresh_ar();

	  virtual void apply_raw_damage_hundredths( unsigned int damage, Mobile::Character* source, bool userepsys = true, bool send_damage_packet = false );
	  virtual void inform_engaged( Mobile::Character* engaged );
	  virtual void inform_disengaged( Mobile::Character* disengaged );
	  virtual void inform_criminal( Mobile::Character* thecriminal );
	  virtual void inform_leftarea( Mobile::Character* wholeft );
	  virtual void inform_enteredarea( Mobile::Character* whoentered );
	  virtual void inform_moved( Mobile::Character* moved );
	  virtual void inform_imoved( Mobile::Character* chr );
	  virtual bool can_be_renamed_by( const Mobile::Character* chr ) const;
	  bool can_accept_event( EVENTID eventid );
	  Bscript::BObjectImp* send_event( Bscript::BObjectImp* event );
	  Mobile::Character* master( ) const;

	  virtual double armor_absorb_damage( double damage );
	  virtual void get_hitscript_params( double damage,
										 Items::UArmor** parmor,
										 unsigned short* rawdamage );
	  virtual Items::UWeapon* intrinsic_weapon();

	  void restart_script();
	  void start_script();

	  bool could_move( UFACING dir ) const;
	  bool anchor_allows_move( UFACING dir ) const;
	  bool npc_path_blocked( UFACING dir ) const;

	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const; ///id test
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, int value );
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string& );
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value );//id test
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, const std::string& );//id test
	  virtual Bscript::BObjectImp* script_method( const char* methodname, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* custom_script_method( const char* methodname, Bscript::Executor& ex );
	  virtual bool script_isa( unsigned isatype ) const;

	  virtual const char *classname() const;
	  virtual void printOn( Clib::StreamWriter& sw ) const;
	  virtual void printSelfOn( Clib::StreamWriter& sw ) const;
	  virtual void printProperties( Clib::StreamWriter& sw ) const;
	  virtual void printDebugProperties( Clib::StreamWriter& sw ) const;

	  virtual void readProperties( Clib::ConfigElem& elem );
	  void readNpcProperties( Clib::ConfigElem& elem );
	  void loadResistances( int resistanceType, Clib::ConfigElem& elem );
	  void loadDamages( int damageType, Clib::ConfigElem& elem );
	  void readNewNpcAttributes( Clib::ConfigElem& elem );
	  void readPropertiesForNewNPC( Clib::ConfigElem& elem );
	  virtual void destroy();

	  NpcTemplate::ALIGNMENT alignment() const;

	  unsigned short damaged_sound;
	  bool use_adjustments; //DAVE
	  unsigned short run_speed; //DAVE
	protected:
	  UOExecutor *ex;
	  UOExecutor *give_item_ex;

      boost_utils::npctemplate_name_flystring template_name;
	  void stop_scripts();
	  friend class Module::NPCExecutorModule;
	  friend class ref_ptr<NPC>;

	private: // REPUTATION SYSTEM:
	  virtual void repsys_on_attack( Mobile::Character* defender );
	  virtual void repsys_on_damage( Mobile::Character* defender );
	  virtual void repsys_on_help( Mobile::Character* recipient );

	  virtual unsigned char hilite_color_idx( const Mobile::Character* seen_by ) const;
	  virtual unsigned short name_color( const Mobile::Character* seen_by ) const;
	  friend class RepSystem;

	private:
      boost_utils::script_name_flystring script;

	  unsigned short npc_ar_;
      s16 getCurrentResistance( ElementalType type ) const;
      void setCurrentResistance( ElementalType type, s16 value );
      s16 getCurrentElementDamage( ElementalType type ) const;
      void setCurrentElementDamage( ElementalType type, s16 value );

	  void reset_element_resist( ElementalType resist );
      void reset_element_damage( ElementalType damage );

	  CharacterRef master_;
	  const NpcTemplate& template_;
	  struct
	  {
		bool enabled;
		unsigned short x;
		unsigned short y;
		unsigned short dstart;
		unsigned short psub;
	  } anchor;
	  unsigned short speech_color_;
	  unsigned short speech_font_;

	private: // not implemented
	  NPC( const NPC& npc );
	  NPC& operator=( const NPC& npc );
	};

	inline Mobile::Character* NPC::master( ) const
	{
	  return master_.get();
	}

	inline NpcTemplate::ALIGNMENT NPC::alignment() const
	{
	  return template_.alignment;
	}

	inline void npc_spoke( NPC& npc, Mobile::Character *chr, const char *text, int textlen, u8 texttype )
	{
	  // Not happy with this comparison style, but can't think of a better alternative.
	  if ( npc.serial != chr->serial )
		npc.on_pc_spoke( chr, text, texttype );
	}

    inline void npc_spoke( NPC& npc, Mobile::Character *chr, const char *text, int textlen, u8 texttype, const u16 *wtext, const char lang[4], int wtextlen, Bscript::ObjArray* speechtokens = NULL )
	{
	  if ( npc.serial != chr->serial )
		npc.on_pc_spoke( chr, text, texttype, wtext, lang, speechtokens );
	}

	inline unsigned short NPC::ar() const
	{
	  if ( ar_ == 0 )
		return npc_ar_;
	  else
		return ar_;
	}
  }
}
#endif
