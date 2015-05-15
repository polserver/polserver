/*
History
=======
2005/03/09 Shinigami: Added Prop Delay_Mod [ms] for WeaponDelay
2005/11/23 MuadDib:   Added warmode_wait for changing war mode.
2005/12/06 MuadDib:   Added uclang member for storing UC language from client.
2006/05/04 MuadDib:   Removed get_legal_item for no use.
2006/05/16 Shinigami: added Prop Race (RACE_* Constants) to support Elfs
2008/07/08 Turley:    get_flag1() changed to show WarMode of other player again
                      get_flag1_aos() removed
2009/02/01 MuadDib:   Resistance storage added.
2009/09/15 MuadDib:   Cleanup from registered houses on destroy
                      u32 registered_house added to store serial of registered multi.
2009/09/22 MuadDib:   Rewrite for Character/NPC to use ar(), ar_mod(), ar_mod(newvalue) virtuals.
2009/09/22 Turley:    repsys param to applydamage
2009/10/14 Turley:    new priv canbeheardasghost
2009/10/14 Turley:    Added char.deaf() methods & char.deafened member
2009/10/17 Turley:    PrivUpdater for "seehidden", "seeghosts", "seeinvisitems" and "invul" - Tomi
                      fixed "all" priv
2009/11/16 Turley:    added NpcPropagateEnteredArea()/inform_enteredarea() for event on resurrection
2009/11/20 Turley:    RecalcVitals can update single Attributes/Vitals - based on Tomi
2010/01/15 Turley:    (Tomi) priv runwhilestealth
2010/01/22 Turley:    Speedhack Prevention System
2010/02/03 Turley:    MethodScript support for mobiles
2011/11/28 MuadDib:   Removed last of uox referencing code.

Notes
=======

*/

#ifndef __CHARACTR_H
#define __CHARACTR_H

#include "../../clib/passert.h"
#include "../gameclck.h"
#include "../polclock.h"
#include "../uobject.h"
#include "../action.h"
#include "../uconst.h"
#include "../skillid.h"
#include "../utype.h"

#include "../../clib/strset.h"
#include "../reftypes.h"

#include "attribute.h"

#include <ctime>

#include <string>
#include <vector>
#include <map>
#include <set>

namespace Pol {
  namespace Bscript {
	class BObjectImp;
	class EScriptProgram;
    class ObjArray;
  }
  namespace Clib {
	class ConfigElem;
  }
  namespace Accounts {
	class Account;
  }
  namespace Network {
	class Client;
  }
  namespace Items {
	class Item;
	class UArmor;
	class UWeapon;
  }
  namespace Core {
    class RepSystem;
    class MenuItem;
	class Menu;
	class OneShotTask;
	class Spellbook;
	class TargetCursor;
	class UOExecutor;
	class USpell;
	class Vital;
    class SaveContext;
    class Party;
	class Guild;
    struct PKTIN_7D;
	struct PKTIN_00;
	struct PKTIN_8D;
	struct PKTIN_F8;
	void ClientCreateChar( Network::Client* client, PKTIN_00* msg );
    void ClientCreateCharKR( Network::Client* client, PKTIN_8D* msg );
    void ClientCreateChar70160( Network::Client* client, PKTIN_F8* msg );
    void createchar2( Accounts::Account* acct, unsigned index );
    void undo_get_item( Mobile::Character *chr, Items::Item *item );
    void write_characters( SaveContext& sc );
    void write_npcs( SaveContext& sc );
  }
  namespace Module {
	class UOExecutorModule;
  }
  namespace Mobile {
	
	class Attribute;
	class Character;

	class AttributeValue
	{
	public:
	  AttributeValue() : _base( 0 ), _temp( 0 ), _intrinsic( 0 ), _lockstate( 0 ), _cap( 0 ) {}

	  int effective() const
	  {
		int v = _base;
		v += _temp;
		v += _intrinsic;
		return ( v > 0 ) ? ( v / 10 ) : 0;
	  }
	  int effective_tenths() const
	  {
		int v = _base;
		v += _temp;
		v += _intrinsic;
		return ( v > 0 ) ? v : 0;
	  }

	  int base() const
	  {
		return _base;
	  }

	  void base( unsigned short base )
	  {
		passert( base <= ATTRIBUTE_MAX_BASE );
		_base = base;
	  }

	  int temp_mod() const
	  {
		return _temp;
	  }
	  void temp_mod( short temp )
	  {
		_temp = temp;
	  }

	  int intrinsic_mod() const
	  {
		return _intrinsic;
	  }
	  void intrinsic_mod( short val )
	  {
		_intrinsic = val;
	  }

	  unsigned char lock() const
	  {
		return _lockstate;
	  }
	  void lock( unsigned char lockstate )
	  {
		_lockstate = lockstate;
	  }

	  unsigned short cap() const
	  {
		return _cap;
	  }
	  void cap( unsigned short cap )
	  {
		_cap = cap;
	  }

	private:
	  unsigned short _base;
	  short _temp;
	  short _intrinsic; // humm, a vector here?
	  unsigned char _lockstate;
	  unsigned short _cap;
	};

	class VitalValue
	{
	public:
	  VitalValue() : _current( 0 ), _maximum( 0 ), _regenrate( 0 ) {}

	  // accessors:
	  int current() const
	  {
		return _current;
	  }
	  int current_ones() const
	  {
		return _current / 100;
	  }
	  int maximum() const
	  {
		return _maximum;
	  }
	  int maximum_ones() const
	  {
		return _maximum / 100;
	  }
	  bool is_at_maximum() const
	  {
		return ( _current >= _maximum );
	  }
	  int regenrate() const
	  {
		return _regenrate;
	  }

	  // mutators:
	protected:
	  friend class Character;
	  void current( int cur )
	  {
		_current = cur;
		if ( _current > _maximum )
		  current( _maximum );
	  }
	  void current_ones( int ones )
	  {
		_current = ones * 100;
		if ( _current > _maximum )
		  current( _maximum );
	  }
	  void maximum( int val )
	  {
		_maximum = val;
		if ( _current > _maximum )
		  current( _maximum );
	  }
	  void regenrate( int rate )
	  {
		_regenrate = rate;
	  }
	  bool consume( unsigned int hamt )
	  {
		if ( _current >= hamt )
		{
		  _current -= hamt;
		  return true;
		}
		else
		{
		  _current = 0;
		  return false;
		}
	  }
	  void produce( unsigned int hamt )
	  {
		unsigned newcur = _current + hamt;
		if ( newcur > _maximum ||
			 newcur < _current )
		{
		  _current = _maximum;
		}
		else
		{
		  _current = newcur;
		}
	  }

	private:
	  unsigned int _current;     // 0 to 10000000 [0 to 100000.00]
	  unsigned int _maximum;
	  int _regenrate; // in hundredths of points per minute
	};

	struct reportable_t { u32 serial; Core::polclock_t polclock; };
	inline bool operator < ( const reportable_t& lhs, const reportable_t& rhs )
	{
	  return ( lhs.serial < rhs.serial ) ||
		( lhs.serial == rhs.serial && lhs.polclock < rhs.polclock );
	}

	struct CachedSettings
	{
	  CachedSettings() :
		all( false ),
		moveany( false ),
		moveanydist( false ),
		renameany( false ),
		clotheany( false ),
		invul( false ),
		seehidden( false ),
		seeghosts( false ),
		hearghosts( false ),
		seeinvisitems( false ),
		dblclickany( false ),
		losany( false ),
		ignoredoors( false ),
		freemove( false ),
		firewhilemoving( false ),
		attackhidden( false ),
		hiddenattack( false ),
		plogany( false ),
		canbeheardasghost( false ),
		runwhilestealth( false ),
		speedhack( false )
	  {
	  };
	  bool all;
	  bool moveany;    // should everything be moveable?
	  bool moveanydist;
	  bool renameany;  // should everything be renameable?
	  bool clotheany;
	  bool invul;
	  bool seehidden;
	  bool seeghosts;
	  bool hearghosts;
	  bool seeinvisitems;
	  bool dblclickany;
	  bool losany;    // all targetting ignore LOS?
	  bool ignoredoors;
	  bool freemove;
	  bool firewhilemoving;
	  bool attackhidden;
	  bool hiddenattack;
	  bool plogany;
	  bool canbeheardasghost;
	  bool runwhilestealth;
	  bool speedhack;
	};


	// NOTES:
	//  The location of the wornitems container MUST be updated whenever the character
	//  moves.  This is so that range checks for things in his pack will succeed.
	//
	// Consider: make a class, UCreature, derived from UObject.  Character would derive from
	//  this, as might NPC.

	class Character : public Core::UObject
	{
	  // types:
	  typedef UObject base;
	  typedef std::set<Character*> CharacterSet;

	public:
	  explicit Character( u32 objtype, UOBJ_CLASS uobj_class = CLASS_CHARACTER );
	  virtual ~Character();
	private:
	  // non-implemented functions:
	  Character( const Character& );
	  Character& operator=( const Character& );

	// UOBJECT INTERFACE
	public:
      virtual size_t estimatedSize( ) const POL_OVERRIDE;
	  
	  virtual void destroy() POL_OVERRIDE;
	  virtual u8 los_height() const POL_OVERRIDE;
	  virtual unsigned int weight() const POL_OVERRIDE;
	  
	  virtual bool setgraphic( u16 newobjtype ) POL_OVERRIDE;
	  virtual void on_color_changed() POL_OVERRIDE;
	  virtual void setfacing( u8 newfacing ) POL_OVERRIDE;
	  virtual void on_facing_changed() POL_OVERRIDE;
	  
	  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;
	  
	  virtual Bscript::BObjectImp* make_ref() POL_OVERRIDE;

	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE; //id test
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string& value ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, int value ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, const std::string& value ) POL_OVERRIDE;//id test
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ) POL_OVERRIDE;//id test
	  virtual Bscript::BObjectImp* set_script_member_id_double( const int id, double value ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* script_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* custom_script_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
	  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;
	  virtual const char* target_tag() const POL_OVERRIDE;
	protected:
	  virtual const char *classname() const POL_OVERRIDE;
	  virtual void printOn( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual void printSelfOn( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual void printDebugProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;

	public:
	  Bscript::BObjectImp* make_offline_ref();

	// NPC INTERFACE
	public:
	  virtual Items::UWeapon* intrinsic_weapon( );

	  virtual void inform_disengaged( Character* disengaged );
	  virtual void inform_engaged( Character* engaged );
	  virtual void inform_criminal( Character* thecriminal );
	  virtual void inform_leftarea( Character* wholeft );
	  virtual void inform_enteredarea( Character* whoentered );
	  virtual void inform_moved( Character* moved );
	  virtual void inform_imoved( Character* chr );
	  virtual double armor_absorb_damage( double damage );
	  virtual void get_hitscript_params( double damage,
										 Items::UArmor** parmor,
										 unsigned short* rawdamage );
	  virtual unsigned short ar() const;
	  virtual void refresh_ar();

	  virtual void apply_raw_damage_hundredths( unsigned int damage, Character* source, bool userepsys = true, bool send_damage_packet = false );
	  void on_swing_failure( Character* attacker );
	  virtual void on_death( Items::Item* corpse );

	  virtual bool can_be_renamed_by( const Character* chr ) const;

	  virtual void repsys_on_attack( Character* defender );
	  virtual void repsys_on_damage( Character* defender );
	  virtual void repsys_on_help( Character* recipient );
	  virtual unsigned char hilite_color_idx( const Character* seen_by ) const;
	  virtual unsigned short name_color( const Character* seen_by ) const;
	protected:
	  virtual u16 get_damaged_sound() const;

	// EQUIPMENT / ITEMS
	public:
	  unsigned short carrying_capacity() const;
	  bool layer_is_equipped( int layer ) const;

	  bool strong_enough_to_equip( const Items::Item* item ) const;
	  bool equippable( const Items::Item* item ) const;
	  bool is_equipped( const Items::Item* item ) const;
	  void equip( Items::Item *item ); // You MUST check equippable() before calling this
	  void unequip( Items::Item *item );

	  Core::Spellbook* spellbook( u8 school ) const;
	  Core::UContainer *backpack( ) const;
	  Items::Item* wornitem( int layer ) const;
	  unsigned int gold_carried() const;
	  void spend_gold( unsigned int amount );

	  void clear_gotten_item();

	  void add_remote_container( Items::Item* );
	  Items::Item* search_remote_containers( u32 serial, bool* isRemoteContainer ) const;
	  bool mightsee( const Items::Item *item ) const;

	  Items::Item *find_wornitem( u32 serial ) const;
	  bool has_shield() const;
	  Items::UArmor* get_shield() const;

	// MOVEMENT
	public:
	  bool on_mount() const;
	  static Core::MOVEMODE decode_movemode( const std::string& str );
	  static std::string encode_movemode( Core::MOVEMODE movemode );
	  // if a move were made, what would the new position be?
	  void getpos_ifmove( Core::UFACING i_facing, unsigned short* px, unsigned short* py );
	  bool can_face( Core::UFACING i_facing );
	  bool face( Core::UFACING i_facing, int flags = 0 );
	  bool move( unsigned char dir );
	  bool CustomHousingMove( unsigned char i_dir );
	  void tellmove( void );
	  void check_region_changes();
	  void check_weather_region_change( bool force = false );
	  void check_light_region_change();
	  void check_justice_region_change();
	  void check_music_region_change();
	  void realm_changed();

	  bool CheckPushthrough();
	  // KLUDGE - a more foolproof way is needed to keep this in sync.
	  void position_changed( void );

	// COMBAT
	public:
	  void select_opponent( u32 serial );
	  void set_opponent( Character* opponent, bool inform_old_opponent = true );
	  
	  void clear_opponent_of();

	  void send_warmode();
	  unsigned short get_weapon_skill() const;
	  Core::USKILLID weapon_skillid() const;
	  const AttributeValue& weapon_attribute() const;

	  Core::UACTION weapon_anim() const;
	  unsigned short random_weapon_damage() const;
	  unsigned short min_weapon_damage() const;
	  unsigned short max_weapon_damage() const;
	  void damage_weapon();
	  void do_attack_effects( Character* target );
	  void do_imhit_effects();
	  void do_hit_success_effects();
	  void do_hit_failure_effects();

	  bool is_attackable( Character* who ) const;
	  Character* get_opponent() const;
	  Character* get_attackable_opponent() const;

	  Items::UArmor* choose_armor( ) const;
	  
	  void showarmor() const;

	  void reset_swing_timer();
	  void check_attack_after_move();
	  void attack( Character* opponent );
	  void send_highlight() const;
	  bool manual_set_swing_timer( int time );

	  const CharacterSet& hostiles() const;
	  void run_hit_script( Character* defender, double damage );

	private:
	  void schedule_attack();
	  static void swing_task_func( Character* chr );

	// ATTRIBUTES / VITALS
	public:
	  void disable_regeneration_for( int seconds );
	  void refresh_element( Core::ElementalType element );
      void update_element( Core::ElementalType element, Items::Item *item );
      s16 calc_element_resist( Core::ElementalType resist ) const;
      s16 calc_element_damage( Core::ElementalType element ) const;

	  u16 strength() const;
	  u16 intelligence() const;
	  u16 dexterity() const;

	  u16 max_hits() const;
	  u32 max_hits_hundredths() const;
	  u16 max_mana() const;
	  u16 max_stamina() const;

	  void set_strength( u16 strength );
	  void set_intelligence( u16 intelligence );
	  void set_dexterity( u16 dexterity );
	  void validate_stat_ranges();

	  double apply_damage( double damage, Character* source = NULL, bool userepsys = true, bool send_damage_packet = false );
	  void heal_damage_hundredths( unsigned int damage );

	  void resurrect();
	  void die();
	  bool dead() const;
	  bool check_skill( Core::USKILLID skillid, int difficulty, unsigned short pointvalue );
	  void award_raw_skillpoints( Core::USKILLID skillid, unsigned short points );
	  unsigned short get_skill( Core::USKILLID skillid ) const;
	  unsigned short get_base_skill( Core::USKILLID skillid ) const;
	  unsigned int get_raw_skill( Core::USKILLID skillid ) const;
	  void set_raw_skill( Core::USKILLID skillid, unsigned int raw_value );

	  void recalc_skill( int skillnum );

	  const AttributeValue& attribute( unsigned attrid ) const;
	  AttributeValue& attribute( unsigned attrid );
	  void set_caps_to_default();

	  const VitalValue& vital( unsigned vitalid ) const;
	  VitalValue& vital( unsigned vitalid );
	  void regen_vital( const Core::Vital* ); // throw()
	  void calc_vital_stuff( bool i_mod = true, bool v_mod = true ); // throw()
	  void calc_single_vital( const Core::Vital* pVital );
	  void calc_single_attribute( const Attribute* pAttr );
	  void set_vitals_to_maximum(); // throw();
	  void produce( const Core::Vital* pVital, VitalValue& vv, unsigned int amt );
	  bool consume( const Core::Vital* pVital, VitalValue& vv, unsigned int amt );
	  void set_current_ones( const Core::Vital* pVital, VitalValue& vv, unsigned int ones );
	  void set_current( const Core::Vital* pVital, VitalValue& vv, unsigned int ones );

	// REPUTATION
	public:
	  friend class Core::RepSystem;

	  Bscript::ObjArray* GetReportables() const;
	  Bscript::ObjArray* GetAggressorTo( ) const;
	  Bscript::ObjArray* GetLawFullyDamaged( ) const;

	  bool is_aggressor_to( const Character* chr ) const;
	  void restart_aggressor_timer( Character* chr, Core::polclock_t until );
	  void remove_as_aggressor_to( Character* chr );

	  bool has_lawfully_damaged( const Character* chr ) const;
	  void restart_lawfully_damaged_timer( Character* chr, Core::polclock_t until );
	  void remove_as_lawful_damager( Character* chr );

	  bool is_criminal() const;
	  bool is_temporally_criminal() const;
	  void restart_criminal_timer( Core::polclock_t until );
	  void clear_criminal_timer();

	  bool is_murderer() const;
	  bool is_innocent_to( const Character* chr ) const;
	  void make_criminal( int level = 1 );
	  void make_murderer( bool newlval = true );
	  void make_aggressor_to( Character* chr );
	  void make_lawfullydamaged_to( Character* chr );

	  void add_to_be_reportable( u32 serial );
	  void clear_to_be_reportables();
	  void commit_to_reportables();
	  void clear_reportable( u32 serial, Core::polclock_t time );
	  void clear_my_aggressors();
	  void clear_my_lawful_damagers();
	  void check_undamaged();

	  void on_criminal_changed();
	  void on_murderer_changed();
	  void on_aggressor_changed();
	  void on_lawfullydamaged_changed();

	// GUILD
	public:
	  Core::Guild* guild() const;
	  void guild( Core::Guild* );
	  unsigned int guildid() const;
	  bool is_guild_ally( const Character* chr ) const;
	  bool is_guild_enemy( const Character* chr ) const;

	// PARTY
	public:
	  Core::Party* party() const;
	  void party( Core::Party* );
	  Core::Party* candidate_of( ) const;
	  void candidate_of( Core::Party* );
	  Core::Party* offline_mem_of( ) const;
	  void offline_mem_of( Core::Party* );
	  bool party_can_loot() const;
	  void set_party_can_loot( bool );
	  void set_party_invite_timeout();
	  bool has_party_invite_timeout() const;
	  void cancel_party_invite_timeout();

    // SECURE TRADING
	public:
	  bool is_trading() const;
	  void create_trade_container();
	  Core::UContainer* trade_container( );

	// SCRIPT
	public:
	  void schedule_spell( Core::USpell* );
	  bool casting_spell() const;
	  bool skill_ex_active() const;
	  bool start_script( Bscript::EScriptProgram* prog,
						 bool start_attached,
						 Bscript::BObjectImp* param2 = NULL,
						 Bscript::BObjectImp* param3 = NULL,
						 Bscript::BObjectImp* param4 = NULL );
	  bool start_skill_script( Bscript::EScriptProgram* prog );
	  bool start_itemuse_script( Bscript::EScriptProgram* prog, Items::Item* item, bool start_attached );
	  bool start_spell_script( Bscript::EScriptProgram* prog, Core::USpell* spell );
	  void cancel_menu();
	private:
	  friend void handle_script_cursor( Character* chr, UObject* obj );
	  friend void menu_selection_made( Network::Client* client, Core::MenuItem* mi, Core::PKTIN_7D* msg );
	  friend class Module::UOExecutorModule;
	  void stop_skill_script();

	// CLIENT 
	public:
	  bool has_active_client() const;
	  bool has_active_prompt() const;
	  bool has_active_gump() const;
	  bool is_house_editing() const;
	  bool target_cursor_busy() const;
	  u16 last_textcolor() const;
      void last_textcolor( u16 color );

	  u8 get_flag1( Network::Client *client ) const;

	// PRIVS SETTINGS STATUS
	public:
	  void on_aos_ext_stat_changed();
	  void on_cmdlevel_changed();
	  void on_poison_changed();
	  void on_hidden_changed();
	  void on_concealed_changed();

	  void set_warmode( bool warmode );
	  void set_stealthsteps( unsigned short newval );
	  bool doors_block() const;
	  bool ignores_line_of_sight() const;

	  bool is_visible() const; // meant to combine "hiding", "concealed", "invisible" etc.
	  bool is_visible_to_me( const Character* chr ) const;
	  bool hidden() const;
	  void hidden( bool value );
	  void unhide();
	  unsigned char concealed() const;
	  void concealed( unsigned char value );
	  bool is_concealed_from_me( const Character* chr ) const;
	  bool invul() const;
	  bool frozen() const;
	  bool paralyzed() const;
	  bool squelched() const;
	  bool deafened() const;
	  bool poisoned() const;
	  void poisoned( bool value );
	  unsigned char cmdlevel() const;
	  void cmdlevel( unsigned char value, bool update_on_change = true );

	  bool can_move( const Items::Item* item ) const;
	  bool can_rename( const Character* chr ) const;
	  bool can_clothe( const Character* chr ) const;
	  bool can_hearghosts() const;
	  bool can_be_heard_as_ghost() const;
	  bool can_seeinvisitems() const;
	  bool can_dblclickany() const;
	  bool can_moveanydist() const;
	  bool can_plogany() const;
	  bool can_speedhack() const;

	  bool has_privilege( const char* priv ) const;
	  bool setting_enabled( const char* setting ) const;
	  void grant_privilege( const char* priv );
	  void revoke_privilege( const char* priv );
	  void set_setting( const char* setting, bool value );
	  std::string all_settings() const;
	  std::string all_privs() const;
	  void set_privs( const std::string& privlist );

	  void check_concealment_level();
	private:
	  void refresh_cached_settings( bool update = true );

	// SERIALIZATION
	public:
	  void readCommonProperties( Clib::ConfigElem& elem );
	  void readAttributesAndVitals( Clib::ConfigElem& elem );
	protected:
	  friend void Core::write_characters( Core::SaveContext& sc );
	  friend void Core::write_npcs( Core::SaveContext& sc );

	  void printWornItems( Clib::StreamWriter& sw_pc, Clib::StreamWriter& sw_equip ) const;

	// CREATION
	private:
	  friend void Core::ClientCreateChar( Network::Client* client, Core::PKTIN_00* msg );
      friend void Core::ClientCreateCharKR( Network::Client* client, Core::PKTIN_8D* msg );
      friend void Core::ClientCreateChar70160( Network::Client* client, Core::PKTIN_F8* msg );
      friend void Core::createchar2( Accounts::Account* acct, unsigned index );
	
	// MISC
	public:
	  void removal_cleanup();
	  void disconnect_cleanup();
	  int charindex() const; // find account character index, or -1 if not found.
	  void on_delete_from_account();
	protected:
	  friend void Core::undo_get_item( Character *chr, Items::Item *item ); // this just gets uglier and uglier.

	// ==========================================================
	// DATA:
	// ==========================================================
	// UOBJECT INTERFACE

	// NPC INTERFACE
	protected:
	// EQUIPMENT / ITEMS
	protected:
      DYN_PROPERTY(carrying_capacity_mod, s16, Core::PROP_CARRY_CAPACITY_MOD, 0);
      
	  Items::UWeapon* weapon;
	  Items::UArmor* shield;
	  std::vector<Items::UArmor*> armor_;

	  ref_ptr<Core::WornItemsContainer> wornitems_ref;
	  Core::WornItemsContainer& wornitems;
	public:
	  Items::Item* gotten_item;
	  enum GOTTEN_ITEM_TYPE : u8
	  {
		GOTTEN_ITEM_ON_GROUND,
		GOTTEN_ITEM_EQUIPPED_ON_SELF,
		GOTTEN_ITEM_IN_CONTAINER
	  } gotten_item_source;

	  std::vector< Core::ItemRef > remote_containers_; // does not own its objects
	// MOVEMENT
	public:
	  u8 dir;				// the entire 'dir' from their last MSG02_WALK 
	  short  gradual_boost;
	  u16 lastx, lasty;	// position before their last MSG02_WALK 
	  s8 lastz;

	  enum MOVEREASON : u8 { WALKED = 0, OTHER = 0, MULTIMOVE = 1 } move_reason;
	  Core::MOVEMODE movemode;
      DYN_PROPERTY(lightoverride,       int,               Core::PROP_LIGHTOVERRIDE,       -1);
      DYN_PROPERTY(lightoverride_until, Core::gameclock_t, Core::PROP_LIGHTOVERRIDE_UNTIL, 0);
      
      static const Core::MovementCostMod DEFAULT_MOVEMENTCOSTMOD;
      DYN_PROPERTY(movement_cost, Core::MovementCostMod, Core::PROP_MOVEMENTCOST_MOD, DEFAULT_MOVEMENTCOSTMOD);
	// COMBAT
	public:
	  u32 warmode_wait;
	protected:
	  u16 ar_;
      DYN_PROPERTY(ar_mod,            s16, Core::PROP_AR_MOD,            0);
      DYN_PROPERTY(delay_mod,         s16, Core::PROP_DELAY_MOD,         0);
      DYN_PROPERTY(hitchance_mod,     s16, Core::PROP_HITCHANCE_MOD,     0);
      DYN_PROPERTY(evasionchance_mod, s16, Core::PROP_EVASIONCHANCE_MOD, 0);

	  Character* opponent_;
	  CharacterSet opponent_of;
	  Core::polclock_t swing_timer_start_clock_;
	  bool ready_to_swing;
	  Core::OneShotTask* swing_task;
	// ATTRIBUTES / VITALS
	public:
	  time_t disable_regeneration_until;
	  std::vector<AttributeValue> attributes;
	  std::vector<VitalValue> vitals;
	// REPUTATION
	private: 
	  typedef std::map< Core::CharacterRef, Core::polclock_t > MobileCont;
	  typedef std::set<reportable_t> ReportableList;
	  typedef std::set<USERIAL> ToBeReportableList;
	  bool murderer_;
	  
	  mutable MobileCont aggressor_to_;
	  mutable MobileCont lawfully_damaged_;
	  Core::polclock_t criminal_until_;
	  Core::OneShotTask* repsys_task_;
	  ToBeReportableList to_be_reportable_;
	  ReportableList reportable_;
	// GUILD
	private: 
	  Core::Guild* guild_;
	// PARTY
	private: 
	  Core::Party* party_;
	  Core::Party* candidate_of_;
	  Core::Party* offline_mem_of_;
	  bool party_can_loot_;
	  Core::OneShotTask* party_decline_timeout_;
    // SECURE TRADING
	public:
	  ref_ptr<Core::UContainer> trading_cont;
	  Core::CharacterRef trading_with;
	  bool trade_accepted;
	// SCRIPT
	public:
      DYN_PROPERTY(disable_skills_until, time_t, Core::PROP_DISABLE_SKILLS_UNTIL, 0);
	  Core::TargetCursor* tcursor2;
	  Core::Menu* menu;
	  void( *on_menu_selection )( Network::Client *client, Core::MenuItem *mi, Core::PKTIN_7D *msg );
	protected:
	  Core::UOExecutor* script_ex;
	  Core::OneShotTask* spell_task;
	// CLIENT 
	public:
	  Network::Client* client;
	  bool logged_in;		// for NPCs, this is always true.
	  bool connected;
	  std::string uclang;
	private:
	  u16 _last_textcolor;
	// PRIVS SETTINGS STATUS
	public:
	  u8 cmdlevel_;
	  bool warmode;
	  bool poisoned_;

      static const Core::ExtStatBarFollowers DEFAULT_EXTSTATBARFOLLOWERS;
      static const Core::SkillStatCap DEFAULT_SKILLSTATCAP;
      DYN_PROPERTY(skillstatcap, Core::SkillStatCap,        Core::PROP_STATCAP_SKILLCAP,      DEFAULT_SKILLSTATCAP);
      DYN_PROPERTY(luck,         s16,                       Core::PROP_EXT_STATBAR_LUCK,      0);
      DYN_PROPERTY(followers,    Core::ExtStatBarFollowers, Core::PROP_EXT_STATBAR_FOLLOWERS, DEFAULT_EXTSTATBARFOLLOWERS);
      DYN_PROPERTY(tithing,      s32,                       Core::PROP_EXT_STATBAR_TITHING,   0);

    protected:
	  bool dead_;
	  bool hidden_;
	  u8 concealed_; // 0 to cmdlevel
	  bool frozen_;
	  bool paralyzed_;
	  u16 stealthsteps_;
	  u32 mountedsteps_;

	  Clib::StringSet privs;
	  Clib::StringSet settings;
	  CachedSettings cached_settings;
	  
      DYN_PROPERTY(squelched_until, Core::gameclock_t, Core::PROP_SQUELCHED_UNTIL, 0);
      DYN_PROPERTY(deafened_until,  Core::gameclock_t, Core::PROP_DEAFENED_UNTIL,  0);
	private:
	// SERIALIZATION

	// CREATION
	protected:
	  Core::gameclock_t created_at;
	// MISC
	public:
	  Core::AccountRef acct;
	  u32 registered_house;
	  u16 truecolor;
	  u32 trueobjtype;
	  Core::UGENDER gender;
	  Core::URACE race;
	  u32 last_corpse;

      DYN_PROPERTY(dblclick_wait, u32, Core::PROP_DOUBLECLICK_WAIT, 0);

      DYN_PROPERTY(title_prefix, std::string, Core::PROP_TITLE_PREFIX, "");
      DYN_PROPERTY(title_suffix, std::string, Core::PROP_TITLE_SUFFIX, "");
      DYN_PROPERTY(title_guild,  std::string, Core::PROP_TITLE_GUILD,  "");
      DYN_PROPERTY(title_race,   std::string, Core::PROP_TITLE_RACE,   "");
	};



	inline bool Character::dead() const
	{
	  return dead_;
	}

	inline bool Character::is_visible() const
	{
	  return !( hidden_ || concealed_ );
	}

	inline bool Character::hidden() const
	{
	  return hidden_;
	}

	inline void Character::hidden( bool value )
	{
		if ( value != hidden_ )
		{
			hidden_ = value;
			on_hidden_changed();
		}
	}

	inline unsigned char Character::concealed() const
	{
		return concealed_;
	}

	inline void Character::concealed( unsigned char value )
	{
		if ( concealed_ != value )
		{
			concealed_ = value;
			on_concealed_changed();
		}
	}

	inline bool Character::frozen() const
	{
	  return frozen_;
	}

	inline bool Character::paralyzed() const
	{
	  return paralyzed_;
	}

	inline bool Character::poisoned() const
	{
		return poisoned_;
	}

	inline void Character::poisoned( bool value )
	{
		if ( value != poisoned_ )
		{
			poisoned_ = value;
			on_poison_changed();
		}
	}

	inline unsigned char Character::cmdlevel() const
	{
		return cmdlevel_;
	}

	inline void Character::cmdlevel( unsigned char value, bool update_on_change )
	{
		if ( cmdlevel_ != value )
		{
			cmdlevel_ = value;
            if ( update_on_change )
              on_cmdlevel_changed();
		}
	}

	inline unsigned short Character::ar() const
	{
	  return ar_;
	}

	inline bool Character::skill_ex_active() const
	{
	  return ( script_ex != NULL );
	}

	inline bool Character::casting_spell() const
	{
	  return ( spell_task != NULL );
	}

	inline const Character::CharacterSet& Character::hostiles() const
	{
	  return opponent_of;
	}

	inline bool Character::ignores_line_of_sight() const
	{
	  return cached_settings.losany;
	}
	inline bool Character::can_seeinvisitems() const
	{
	  return cached_settings.seeinvisitems;
	}
	inline bool Character::can_dblclickany() const
	{
	  return cached_settings.dblclickany;
	}

	inline bool Character::has_shield() const
	{
		return (shield != NULL);
	}

	inline Items::UArmor* Character::get_shield() const
	{
		return shield;
	}

	inline const AttributeValue& Character::attribute( unsigned attrid ) const
	{
	  passert( attrid < attributes.size() );
	  return attributes[attrid];
	}
	inline AttributeValue& Character::attribute( unsigned attrid )
	{
	  passert( attributes.size() > attrid );
	  return attributes[attrid];
	}
	inline const VitalValue& Character::vital( unsigned vitalid ) const
	{
	  passert( vitals.size() > vitalid );
	  return vitals[vitalid];
	}
	inline VitalValue& Character::vital( unsigned vitalid )
	{
	  passert( vitalid < vitals.size() );
	  return vitals[vitalid];
	}

	//dave moved this here from .cpp 2/3/3 so i can use it in uoemod.cpp
	inline void NpcPropagateMove( Character* chr, Character* moved )
	{
	  if ( chr != moved )
	  {
		chr->inform_moved( moved );
		moved->inform_imoved( chr );
	  }
	}

	inline void NpcPropagateCriminal( Character* chr, Character* thecriminal )
	{
	  if ( chr != thecriminal )
		chr->inform_criminal( thecriminal );
	}

	inline void NpcPropagateLeftArea( Character* chr, Character* wholeft )
	{
	  if ( chr != wholeft )
	  {
		chr->inform_leftarea( wholeft );
	  }
	}

	inline void NpcPropagateEnteredArea( Character* chr, Character* whoentered )
	{
	  if ( chr != whoentered )
	  {
		chr->inform_enteredarea( whoentered );
	  }
	}
  }
}
#endif
