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

#include "eventid.h"
#include "npctmpl.h"
#include "uoscrobj.h"

#include <iosfwd>
#include <map>
#include <string>

namespace Pol {
  namespace Clib {
	class ConfigElem;
  }
  namespace Module {
    class NPCExecutorModule;
  }
  namespace Core {

	struct Anchor
	{
	  Anchor():
		enabled(false),
		x(0),
		y(0),
		dstart(0),
		psub(0)
	  {};
	  bool enabled;
	  unsigned short x;
	  unsigned short y;
	  unsigned short dstart;
	  unsigned short psub;
	};

	class UOExecutor;
	
	class RepSystem;

	class NPC : public Mobile::Character
	{
	  typedef Mobile::Character base;
	public:
	  explicit NPC( u32 objtype, const Clib::ConfigElem& elem );
      virtual ~NPC();
	private: // not implemented
	  NPC( const NPC& npc );
	  NPC& operator=( const NPC& npc );

	// UOBJECT INTERFACE
	public:
      virtual size_t estimatedSize() const POL_OVERRIDE;

	  virtual void destroy() POL_OVERRIDE;

	  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;

	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE; ///id test
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, int value ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string& ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ) POL_OVERRIDE;//id test
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, const std::string& ) POL_OVERRIDE;//id test
	  virtual Bscript::BObjectImp* script_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* custom_script_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;

	protected:
	  virtual const char *classname() const POL_OVERRIDE;
	  virtual void printOn( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual void printSelfOn( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual void printDebugProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;

	// NPC INTERFACE
	public:
	  virtual Items::UWeapon* intrinsic_weapon() POL_OVERRIDE;

	  virtual void inform_disengaged( Mobile::Character* disengaged ) POL_OVERRIDE;
	  virtual void inform_engaged( Mobile::Character* engaged ) POL_OVERRIDE;
	  virtual void inform_criminal( Mobile::Character* thecriminal ) POL_OVERRIDE;
	  virtual void inform_leftarea( Mobile::Character* wholeft ) POL_OVERRIDE;
	  virtual void inform_enteredarea( Mobile::Character* whoentered ) POL_OVERRIDE;
	  virtual void inform_moved( Mobile::Character* moved ) POL_OVERRIDE;
	  virtual void inform_imoved( Mobile::Character* chr ) POL_OVERRIDE;
	  virtual double armor_absorb_damage( double damage ) POL_OVERRIDE;
	  virtual void get_hitscript_params( double damage,
										 Items::UArmor** parmor,
										 unsigned short* rawdamage ) POL_OVERRIDE;
	  virtual unsigned short ar() const POL_OVERRIDE;
	  virtual void refresh_ar() POL_OVERRIDE;

	  virtual void apply_raw_damage_hundredths( unsigned int damage, Mobile::Character* source, bool userepsys = true, bool send_damage_packet = false ) POL_OVERRIDE;
	  virtual void on_death( Items::Item* corpse ) POL_OVERRIDE;

	  virtual bool can_be_renamed_by( const Mobile::Character* chr ) const POL_OVERRIDE;

	  virtual void repsys_on_attack( Mobile::Character* defender ) POL_OVERRIDE;
	  virtual void repsys_on_damage( Mobile::Character* defender ) POL_OVERRIDE;
	  virtual void repsys_on_help( Mobile::Character* recipient ) POL_OVERRIDE;
	  virtual unsigned char hilite_color_idx( const Mobile::Character* seen_by ) const POL_OVERRIDE;
	  virtual unsigned short name_color( const Mobile::Character* seen_by ) const POL_OVERRIDE;
	protected:
	  virtual u16 get_damaged_sound() const POL_OVERRIDE;
	// MOVEMENT
	public:
	  bool could_move( UFACING dir ) const;
	  bool anchor_allows_move( UFACING dir ) const;
	  bool npc_path_blocked( UFACING dir ) const;
	  
	// EVENTS
	public:
	  // Speech: ASCII versions
	  void on_pc_spoke( Mobile::Character *src_chr, const char *speech, u8 texttype );
	  void on_ghost_pc_spoke( Mobile::Character* src_chr, const char* speech, u8 texttype );
	  // Speech: Unicode (and ASCII) versions
	  void on_pc_spoke( Mobile::Character *src_chr, const char *speech, u8 texttype,
								const u16* wspeech, const char lang[4], Bscript::ObjArray* speechtokens = NULL );
	  void on_ghost_pc_spoke( Mobile::Character* src_chr, const char* speech, u8 texttype,
                                      const u16* wspeech, const char lang[4], Bscript::ObjArray* speechtokens = NULL );
	  bool can_accept_event( EVENTID eventid );
	  Bscript::BObjectImp* send_event_script( Bscript::BObjectImp* event );
	  bool send_event( Bscript::BObjectImp* event );

	// SCRIPT
	public:
	  void restart_script();
	  void start_script();
	protected:
	  void stop_scripts();

	// SERIALIZATION
	public:
	  void readPropertiesForNewNPC( Clib::ConfigElem& elem );
	private:
	  void readNpcProperties( Clib::ConfigElem& elem );
	  void loadResistances( int resistanceType, Clib::ConfigElem& elem );
	  void loadDamages( int damageType, Clib::ConfigElem& elem );
	  void readNewNpcAttributes( Clib::ConfigElem& elem );
    
	// ATTRIBUTES / VITALS
	private:
	  s16 getCurrentResistance( ElementalType type ) const;
      void setCurrentResistance( ElementalType type, s16 value );
      s16 getCurrentElementDamage( ElementalType type ) const;
      void setCurrentElementDamage( ElementalType type, s16 value );

	  void reset_element_resist( ElementalType resist );
      void reset_element_damage( ElementalType damage );

	// MISC
	public:
	  Mobile::Character* master( ) const;
	  NpcTemplate::ALIGNMENT alignment() const;

	// ==========================================================
	// DATA:
	// ==========================================================
	  friend class RepSystem;
	  friend class Module::NPCExecutorModule;
	  friend class ref_ptr<NPC>;

	// UOBJECT INTERFACE
	// NPC INTERFACE
	private:
	  unsigned short npc_ar_;
	// MOVEMENT
	public:
	  unsigned short run_speed;
	  bool use_adjustments;
	private:
	  Anchor anchor;
	// EVENTS
	private:
	  unsigned short speech_color_;
	  unsigned short speech_font_;
	// SCRIPT
	private:
      boost_utils::script_name_flystring script;
	protected:
	  UOExecutor *ex;
	// SERIALIZATION
	// ATTRIBUTES / VITALS
	// MISC
	public:
	  unsigned short damaged_sound;
	protected:
	  boost_utils::npctemplate_name_flystring template_name;
	private:
	  CharacterRef master_;
	  const NpcTemplate& template_;
	};

	inline Mobile::Character* NPC::master( ) const
	{
	  return master_.get();
	}

	inline NpcTemplate::ALIGNMENT NPC::alignment() const
	{
	  return template_.alignment;
	}

	inline void npc_spoke( NPC& npc, Mobile::Character *chr, const char *text, int /*textlen*/, u8 texttype )
	{
	  // Not happy with this comparison style, but can't think of a better alternative.
	  if ( npc.serial != chr->serial )
		npc.on_pc_spoke( chr, text, texttype );
	}

    inline void npc_spoke( NPC& npc, Mobile::Character *chr, const char *text, int /*textlen*/, u8 texttype, const u16 *wtext, const char lang[4], int /*wtextlen*/, Bscript::ObjArray* speechtokens = NULL )
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
