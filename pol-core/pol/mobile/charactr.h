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

Notes
=======

*/

#ifndef __CHARACTR_H
#define __CHARACTR_H

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#include <time.h>

#include "../gameclck.h"
#include "../polclock.h"
#include "../eventid.h"

#ifndef __UOBJECT_H
#	include "../uobject.h"
#endif

#ifndef __CONTAINR_H
#	include "../containr.h"
#endif

#include "../action.h"
#include "../uconst.h"
#include "../layers.h"
#include "../skillid.h"
#include "../utype.h"

#include "../../clib/strset.h"
#include "../reftypes.h"

#include "../ssopt.h"
#include "../multi/house.h"

class Account;
class Attribute;
class BObjectImp;
class Character;
class Client;
class ConfigElem;
class EScriptProgram;
class Item;
class Menu;
class OneShotTask;
class Spellbook;
class TargetCursor;
class UArmor;
class UOExecutor;
class UOExecutorModule;
class USpell;
class UWeapon;
class Vital;

#include "attribute.h"

class AttributeValue
{
public:
	AttributeValue() : _base(0), _temp(0), _intrinsic(0), _lockstate(0), _cap(0) {}

    long effective() const
    {
        long v = _base;
        v += _temp;
        v += _intrinsic;
        return (v>0)?(v/10):0;
    }
    long effective_tenths() const
    {
        long v = _base;
        v += _temp;
        v += _intrinsic;
        return (v>0)?v:0;
    }

    long base() const
    {
        return _base;
    }

    void base( unsigned short base )
    {
        passert( base <= ATTRIBUTE_MAX_BASE );
        _base = base;
    }

    long temp_mod() const
    {
        return _temp;
    }
    void temp_mod( short temp )
    {
        _temp = temp;
    }

    long intrinsic_mod() const
    {
        return _intrinsic;
    }
    void intrinsic_mod( short val )
    {
        _intrinsic = val;
    }

	unsigned char lock () const
	{
		return _lockstate;
	}
	void lock( unsigned char lockstate )
	{
		_lockstate = lockstate;
	}

	unsigned short cap () const
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
    VitalValue() : _current(0), _maximum(0), _regenrate(0) {}

// accessors:
    long current() const
    {
        return _current;
    }
    long current_ones() const
    {
        return _current/100;
    }
    long maximum() const
    {
        return _maximum;
    }
    long maximum_ones() const
    {
        return _maximum/100;
    }
    bool is_at_maximum() const
    {
        return (_current >= _maximum);
    }
    long regenrate() const
    {
        return _regenrate;
    }

// mutators:
protected:
    friend class Character;
    void current( long cur )
    {
        _current = cur;
        if (_current > _maximum)
            current( _maximum );
    }
    void current_ones( long ones )
    {
        _current = ones*100;
        if (_current > _maximum)
            current( _maximum );
    }
    void maximum( long val )
    {
        _maximum = val;
        if (_current > _maximum)
            current( _maximum );
    }
    void regenrate( long rate )
    {
        _regenrate = rate;
    }
    bool consume( unsigned long hamt )
    {
        if (_current >= hamt)
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
    void produce( unsigned long hamt )
    {
        unsigned newcur = _current + hamt;
        if (newcur > _maximum ||
            newcur < _current)
        {
            _current = _maximum;
        }
        else
        {
            _current = newcur;
        }
    }

private:
    unsigned long _current;     // 0 to 10000000 [0 to 100000.00]
    unsigned long _maximum;
    long _regenrate; // in hundredths of points per minute
};

struct reportable_t { u32 serial; polclock_t polclock; };
inline bool operator < (const reportable_t& lhs, const reportable_t& rhs )
{
    return (lhs.serial < rhs.serial) ||
            (lhs.serial == rhs.serial && lhs.polclock < rhs.polclock);
}

bool DecodeFacing( const char* dir, UFACING& facing );


//ok:#include "vital.h"
// NOTES:
//  The location of the wornitems container MUST be updated whenever the character
//  moves.  This is so that range checks for things in his pack will succeed.
//
// Consider: make a class, UCreature, derived from UObject.  Character would derive from
//  this, as might NPC.

class Character : public UObject
{
    typedef UObject base;
public:
    explicit Character( u16 objtype, UOBJ_CLASS uobj_class = CLASS_CHARACTER );
    virtual ~Character();


    // types:
    typedef set<Character*> CharacterSet;


    void removal_cleanup();
    void disconnect_cleanup();

    virtual void destroy();

    virtual u8 los_height() const;
    virtual unsigned long weight() const;
    unsigned short carrying_capacity() const;
    bool layer_is_equipped( int layer ) const;
    static bool valid_equip_layer( const Item* item );

    bool strong_enough_to_equip( const Item* item ) const;
    bool equippable( const Item* item ) const;
    bool is_equipped( const Item* item ) const;
    void equip( Item *item ); // You MUST check equippable() before calling this
    void unequip( Item *item );
    virtual UWeapon* intrinsic_weapon();

    bool on_mount() const;

    bool has_active_client() const;
	bool has_active_prompt() const;
	bool has_active_gump() const;

    Spellbook* spellbook(u8 school) const;
    UContainer *backpack() const;
    Item* wornitem( int layer ) const;
    unsigned long gold_carried() const;
    void spend_gold( unsigned long amount );

    const CharacterSet& hostiles() const;

    void clear_gotten_item();

    bool target_cursor_busy() const;

protected: 
    friend void undo_get_item( Character *chr, Item *item ); // this just gets uglier and uglier.

public:
    
    //    COMBAT FUNCTIONS
    void disable_regeneration_for( int seconds );
    void set_opponent( Character* opponent, bool inform_old_opponent = true );
    virtual void inform_disengaged( Character* disengaged );
    virtual void inform_engaged( Character* engaged );
    virtual void inform_criminal( Character* thecriminal );
	virtual void inform_leftarea( Character* wholeft );
    virtual void inform_enteredarea( Character* whoentered );
	virtual void inform_moved( Character* moved );
    virtual void inform_imoved( Character* chr );
    void clear_opponent_of();
	unsigned long warmode_wait;
	void send_warmode();
    unsigned short get_weapon_skill() const;
    USKILLID weapon_skillid() const;
    const AttributeValue& weapon_attribute() const;

    UACTION weapon_anim() const;
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

    UArmor* choose_armor() const;
    virtual double armor_absorb_damage( double damage );
    virtual void get_hitscript_params( double damage,
                                       UArmor** parmor,
                                       unsigned short* rawdamage );
    void showarmor() const;

    void reset_swing_timer();
    void check_attack_after_move();
    void attack( Character* opponent );
    void send_highlight() const;
	bool manual_set_swing_timer(long time);

    //void find_armor();
    //void calculate_ar();

    virtual unsigned short ar() const;
	virtual s16 ar_mod() const;
	virtual s16 ar_mod( s16 new_value );
	virtual void refresh_ar();
    void refresh_element( unsigned element );
    void update_element( unsigned element, Item *item );
    s16 calc_element_resist( unsigned resist ) const;
    s16 calc_element_damage( unsigned element ) const;

	virtual bool setgraphic( u16 newobjtype );
    virtual void on_color_changed();
	virtual void on_poison_changed(); //dave 12-24
    virtual void setfacing( u8 newfacing );
    virtual void on_facing_changed();
    void on_aos_ext_stat_changed();

    u8 get_flag1(Client *client) const;

    static MOVEMODE decode_movemode( const string& str );
    static string encode_movemode( MOVEMODE movemode );

    void unhide();
    void set_warmode( bool warmode );
    void select_opponent( u32 serial );
    void set_stealthsteps( unsigned short newval );
    
    bool is_visible_to_me( const Character* chr ) const;
    bool is_concealed_from_me( const Character* chr ) const;
    bool doors_block() const;

    // Settings
    bool ignores_line_of_sight() const;
    

    // on_menu_selection MUST be set if menu_id is nonzero.

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

    virtual void apply_raw_damage_hundredths( unsigned long damage, Character* source, bool userepsys = true, bool send_damage_packet = false );
    double apply_damage( double damage, Character* source = NULL, bool userepsys = true, bool send_damage_packet = false );
    void heal_damage_hundredths( unsigned long damage );
    virtual void on_swing_failure( Character* attacker );
    void run_hit_script( Character* defender, double damage );

    void resurrect();
    
    virtual void on_death( Item* corpse );
    void die();
    bool dead() const;
    
    bool is_visible() const; // meant to combine "hiding", "concealed", "invisible" etc.
    bool hidden() const;
    bool concealed() const;
    bool invul() const;
    bool frozen() const;
    bool paralyzed() const;
    bool squelched() const;
    bool deafened() const;

        // if a move were made, what would the new position be?
    void getpos_ifmove( UFACING i_facing, unsigned short* px, unsigned short* py );
    
	bool can_face( UFACING i_facing );
	bool face( UFACING i_facing, long flags = 0 );
    bool move( unsigned char dir );
    void tellmove( void );
    void check_region_changes();
    void check_weather_region_change(bool force=false);
    void check_light_region_change();
    void check_justice_region_change();
    void check_music_region_change();
	void realm_changed();

    void add_remote_container( Item* );
    Item* search_remote_containers( u32 serial, bool* isRemoteContainer ) const;
    bool mightsee( const Item *item ) const;
    Item* get_from_ground( u32 serial, UContainer** found_in );
//	get_legal_item not being used, removed - MuadDib
//	Item* get_legal_item( u32 serial, UContainer** found_in );

    bool can_move( const Item* item ) const;
    bool can_rename( const Character* chr ) const;
    virtual bool can_be_renamed_by( const Character* chr ) const;
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
    string all_settings() const;
    string all_privs() const;
    void set_privs( const string& privlist );

	bool CheckPushthrough();
public: // REPUTATION SYSTEM
    friend class RepSystem;
    
    bool is_aggressor_to( const Character* chr ) const;
    void restart_aggressor_timer( Character* chr, polclock_t until );
    void remove_as_aggressor_to( Character* chr );
    
    bool has_lawfully_damaged( const Character* chr ) const;
    void restart_lawfully_damaged_timer( Character* chr, polclock_t until );
    void remove_as_lawful_damager( Character* chr );

    bool is_criminal() const;
    void restart_criminal_timer( polclock_t until );
    void clear_criminal_timer();

    bool is_murderer() const;
    bool is_innocent_to( const Character* chr ) const;

    virtual void repsys_on_attack( Character* defender );
    virtual void repsys_on_damage( Character* defender );
    virtual void repsys_on_help( Character* recipient ); 
    
    virtual unsigned char hilite_color_idx( const Character* seen_by ) const;
    virtual unsigned short name_color( const Character* seen_by ) const;

    class Guild* guild() const;
    void guild( Guild* );
    unsigned long guildid() const;
    //void guildid( unsigned long gid );

    bool is_guild_ally( const Character* chr ) const;
    bool is_guild_enemy( const Character* chr ) const;
    
    class Party* party() const;
	void party( Party* );
	class Party* candidate_of() const;
	void candidate_of( Party* );
	class Party* offline_mem_of() const;
	void offline_mem_of( Party* );
	bool party_can_loot() const;
	void set_party_can_loot(bool);
	void set_party_invite_timeout();
	bool has_party_invite_timeout();
	void cancel_party_invite_timeout();

    void make_criminal( long level = 1 );
    void make_murderer( bool newlval = true );
	void make_aggressor_to( Character* chr);
	void make_lawfullydamaged_to( Character* chr);

    void add_to_be_reportable( u32 serial );
    void clear_to_be_reportables();
    void commit_to_reportables();
    void clear_reportable( u32 serial, polclock_t time );
    void clear_my_aggressors();
    void clear_my_lawful_damagers();
    void check_undamaged();

public: // SECURE TRADING
    bool is_trading() const;
    void create_trade_container();
    UContainer* trade_container();
    ref_ptr<UContainer> trading_cont;
    CharacterRef trading_with;
    bool trade_accepted;

public:
    /* Item * wornitem( int layer ) const { return wornitems[ layer ]; } */
    Item *find_wornitem( u32 serial ) const;
	    
    // KLUDGE - a more foolproof way is needed to keep this in sync.
    void position_changed( void );

    //bool check_skill( USKILLID skillid );
    bool check_skill( USKILLID skillid, int difficulty, unsigned short pointvalue );
    void award_raw_skillpoints( USKILLID skillid, unsigned short points );
    unsigned short get_skill( USKILLID skillid ) const;
    unsigned short get_base_skill( USKILLID skillid ) const;
    unsigned long get_raw_skill( USKILLID skillid ) const;
    void set_raw_skill( USKILLID skillid, unsigned long raw_value );

    void recalc_all_skills( void );
    void recalc_skill( int skillnum );
    int charindex() const; // find account character index, or -1 if not found.

    void schedule_spell( class USpell* );
    bool casting_spell() const;

    virtual void readProperties( ConfigElem& elem );
    void readCommonProperties( ConfigElem& elem );
    void readAttributesAndVitals( ConfigElem& elem );
    //virtual BObjectImp* script_member( const char *membername );
    virtual BObjectImp* make_ref();
    virtual BObjectImp* make_offline_ref();
    virtual BObjectImp* get_script_member( const char *membername ) const;
    virtual BObjectImp* get_script_member_id( const int id ) const; //id test

    virtual BObjectImp* set_script_member( const char *membername, const std::string& value );
    virtual BObjectImp* set_script_member( const char *membername, long value );
    virtual BObjectImp* set_script_member_id( const int id, const std::string& value );//id test
    virtual BObjectImp* set_script_member_id( const int id, long value );//id test
	virtual BObjectImp* set_script_member_id_double( const int id, double value );

    virtual BObjectImp* script_method( const char* methodname, Executor& ex );
    virtual BObjectImp* script_method_id( const int id, Executor& ex );
	virtual BObjectImp* custom_script_method( const char* methodname, Executor& ex );
    virtual bool script_isa( unsigned isatype ) const;
    virtual const char* target_tag() const;
    
    bool skill_ex_active() const;
    bool start_script( EScriptProgram* prog, 
                       bool start_attached,
                       BObjectImp* param2 = NULL,
                       BObjectImp* param3 = NULL,
					   BObjectImp* param4 = NULL);
    bool start_skill_script( EScriptProgram* prog );
    bool start_itemuse_script( EScriptProgram* prog, Item* item, bool start_attached );
    bool start_spell_script( EScriptProgram* prog, USpell* spell );

    void check_concealment_level();
protected:

    // friend void read_character( ConfigElem& elem );
    friend void uox_read_character( ConfigElem& elem );
    friend void write_characters( class SaveContext& sc );

    void printWornItems( ostream& pc, ostream& equip ) const;

    virtual const char *classname() const;
    virtual void printOn( ostream& os ) const;
    virtual void printSelfOn( ostream& os ) const;
    virtual void printProperties( ostream& os ) const;
    virtual void printDebugProperties( std::ostream& os ) const;

private:
    void schedule_attack();

    static void swing_task_func( Character* chr );



    void refresh_cached_settings( bool update = true );
    void load_default_elements();
	friend void ClientCreateChar( Client* client, struct PKTIN_00* msg);
    friend void ClientCreateChar6017( Client* client, struct PKTIN_8D* msg);
	friend Character* CreateNewBlankCharacter();
	friend void createchar2(Account* acct, unsigned index);

private:
    friend void handle_script_cursor( Character* chr, UObject* obj );
    friend void menu_selection_made( Client* client, class MenuItem* mi, struct PKTIN_7D* msg );

    friend class UOExecutorModule;
    void stop_skill_script();
        
public:
    void cancel_menu();


// DATA:
public:
    AccountRef acct;
    Client* client;

	u32 registered_house; 

    unsigned char cmdlevel;
    u8 dir;				// the entire 'dir' from their last MSG02_WALK 
    bool warmode;
    bool logged_in;		// for NPCs, this is always true.
	bool connected;
    
	u16 lastx, lasty;	// position before their last MSG02_WALK 
    s8 lastz;
	
    enum { WALKED=0, OTHER=0, MULTIMOVE=1 } move_reason;
    MOVEMODE movemode;

    time_t disable_regeneration_until;
    time_t disable_skills_until;

    u16 truecolor;
    u16 trueobjtype;
    UGENDER gender;
    URACE race;
    bool poisoned;
	short  gradual_boost;

	u32 last_corpse;
	unsigned long dblclick_wait;
	Item* gotten_item;
	
	enum { GOTTEN_ITEM_ON_GROUND,
           GOTTEN_ITEM_EQUIPPED_ON_SELF,
           GOTTEN_ITEM_IN_CONTAINER };
    unsigned char gotten_item_source; 

    vector<AttributeValue> attributes;
    vector<VitalValue> vitals;

	string uclang;

	bool has_shield() const
	{
		if (shield != NULL)
			return true;
		else
			return false;
	}

	UArmor* get_shield() const
	{
		if (shield != NULL)
			return shield;
		else
			return NULL;
	}
    // FIXME these should be moved into the protected section
    // these are in hundredths of points
//    s32 hits_;
//    s32 mana_;
//    s32 stamina_;

//    signed short hits_regen_rate_;
//    signed short mana_regen_rate_;
//    signed short stamina_regen_rate_;

//    void set_hits( u16 hits ) { hits_ = hits * 100L; }
//    void set_mana( u16 mana ) { mana_ = mana * 100L; }
//    void set_stamina( u16 stam ) { stamina_ = stam * 100L; }
    
//    void add_mana_hundredths( int add );
//    void add_stamina_hundredths( int add );

//    u16 get_hits() const { return hits_ / 100; }
//    u16 get_mana() const { return mana_ / 100; }
//    u16 get_stamina() const { return stamina_ / 100; }

	TargetCursor* tcursor2;
    Menu* menu;
    void (*on_menu_selection)( Client *client, 
                               class MenuItem *mi, 
                               struct PKTIN_7D *msg );

    int lightoverride;
    gameclock_t lightoverride_until;

    string title_prefix;
    string title_suffix;
    string title_guild;
    string title_race;

	Expanded_Statbar expanded_statbar;
	u16 skillcap_;
	MovementCost_Mod movement_cost;

protected:
    ref_ptr<WornItemsContainer> wornitems_ref;
    WornItemsContainer& wornitems; 

    unsigned short ar_;
    s16 ar_mod_;
    s16 delay_mod_;
	s16 hitchance_mod_;
	s16 evasionchance_mod_;

    UWeapon* weapon;
    UArmor* shield;
    vector<UArmor*> armor_;

protected: // was private, but hey, NPC will be okay, I think.
    bool dead_;
    bool hidden_;
	unsigned char concealed_; // 0 to cmdlevel
    bool frozen_;
    bool paralyzed_;
    unsigned short stealthsteps_;

//    vector< Item* > private_items_; // owns its objects
    vector< ItemRef > remote_containers_; // does not own its objects
    
    StringSet privs;
    StringSet settings;

    struct {
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
    } cached_settings;

    UOExecutor* script_ex;

    Character* opponent_;
    CharacterSet opponent_of;
    polclock_t swing_timer_start_clock_;
    bool ready_to_swing;

    OneShotTask* swing_task;
    OneShotTask* spell_task;


    gameclock_t created_at;
    mutable gameclock_t squelched_until;
    mutable gameclock_t deafened_until;

private: // REPUTATION SYSTEM DATA:
    typedef map< CharacterRef, polclock_t > MobileCont;
    mutable MobileCont aggressor_to_;
    mutable MobileCont lawfully_damaged_;
    polclock_t criminal_until_;
    OneShotTask* repsys_task_;

    typedef set<USERIAL> ToBeReportableList;
    ToBeReportableList to_be_reportable_;

    typedef set<reportable_t> ReportableList;
    ReportableList reportable_;
    ObjArray* GetReportables() const;
    ObjArray* GetAggressorTo() const;
	ObjArray* GetLawFullyDamaged() const;

//    USERIAL guildid_;
    Guild* guild_;
    Party* party_;
	Party* candidate_of_;
	Party* offline_mem_of_;
	bool party_can_loot_;
	OneShotTask* party_decline_timeout_;
    bool murderer_;

    // Language stuff:
//    unsigned langid_;

public:
    const AttributeValue& attribute( unsigned attrid ) const
    {
        passert( attrid < attributes.size() );
        return attributes[attrid];
    }
    AttributeValue& attribute( unsigned attrid )
    {
        passert( attributes.size() > attrid );
        return attributes[attrid];
    }
	void set_caps_to_default();

/* VITALS */
    const VitalValue& vital( unsigned vitalid ) const
    {
        passert( vitals.size() > vitalid );
        return vitals[vitalid];
    }
    VitalValue& vital( unsigned vitalid )
    {
        passert( vitalid < vitals.size() );
        return vitals[vitalid];
    }
    void regen_vital( const Vital* ); // throw()
    void calc_vital_stuff( bool i_mod = true, bool v_mod = true ); // throw()
	void calc_single_vital( const Vital* pVital );
	void calc_single_attribute( const Attribute* pAttr );
    void set_vitals_to_maximum(); // throw();
    void produce( const Vital* pVital, VitalValue& vv, unsigned long amt );
    bool consume( const Vital* pVital, VitalValue& vv, unsigned long amt );
    void set_current_ones( const Vital* pVital, VitalValue& vv, unsigned long ones );
    void set_current( const Vital* pVital, VitalValue& vv, unsigned long ones );

private:
    // non-implemented functions:
    Character( const Character& );
    Character& operator=( const Character& );
};
#include "../vital.h"
  

inline bool Character::dead() const
{
    return dead_;
}

inline bool Character::is_visible() const
{
    return !(hidden_ || concealed_);
}

inline bool Character::hidden() const
{
    return hidden_;
}

inline bool Character::concealed() const
{
    return concealed_?true:false;
}

inline bool Character::frozen() const
{
    return frozen_;
}

inline bool Character::paralyzed() const
{
    return paralyzed_;
}

inline unsigned short Character::ar() const
{
    return ar_;
}

inline s16 Character::ar_mod() const
{
	return ar_mod_;
}

inline s16 Character::ar_mod( s16 new_value )
{
	ar_mod_ = new_value;
	return ar_mod_;
}

inline bool Character::skill_ex_active() const
{
    return (script_ex != NULL);
}

inline bool Character::casting_spell() const
{
    return (spell_task != NULL );
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
//dave moved this here from .cpp 2/3/3 so i can use it in uoemod.cpp
inline void NpcPropagateMove( Character* chr, Character* moved )
{
    if (chr != moved)
    {
        chr->inform_moved( moved );
        moved->inform_imoved( chr );
    }
}

inline void NpcPropagateCriminal( Character* chr, Character* thecriminal )
{
    if (chr != thecriminal)
        chr->inform_criminal( thecriminal );
}

inline void NpcPropagateLeftArea( Character* chr, Character* wholeft )
{
	if (chr != wholeft)
	{
		chr->inform_leftarea( wholeft );
	}
}

inline void NpcPropagateEnteredArea( Character* chr, Character* whoentered )
{
	if (chr != whoentered)
	{
		chr->inform_enteredarea( whoentered );
	}
}

#endif
