/*
History
=======
2006/09/17 Shinigami: send_event() will return error "Event queue is full, discarding event"
2009/09/22 MuadDib:   Rewrite for Character/NPC to use ar(), ar_mod(), ar_mod(newvalue) virtuals.

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

class UOExecutor;
class ConfigElem;
class RepSystem;

class NPC : public Character
{
    typedef Character base;
public:
	explicit NPC( u16 objtype, const ConfigElem& elem );

    virtual void on_death( Item* corpse );
    virtual void on_give_item();

	// Speech: ASCII versions
    virtual void on_pc_spoke( Character *src_chr, const char *speech, u8 texttype);
    virtual void on_ghost_pc_spoke( Character* src_chr, const char* speech, u8 texttype);

	// Speech: Unicode (and ASCII) versions
    virtual void on_pc_spoke( Character *src_chr, const char *speech, u8 texttype,
							  const u16* wspeech, const char lang[4]);
    virtual void on_ghost_pc_spoke( Character* src_chr, const char* speech, u8 texttype,
									const u16* wspeech, const char lang[4]);

	virtual unsigned short ar() const;

    virtual void apply_raw_damage_hundredths( unsigned long damage, Character* source );
    virtual void inform_engaged( Character* engaged );
    virtual void inform_disengaged( Character* disengaged );
    virtual void inform_criminal( Character* thecriminal );
	virtual void inform_leftarea( Character* wholeft );
	virtual void inform_moved( Character* moved );
    virtual void inform_imoved( Character* chr );
    virtual bool can_be_renamed_by( const Character* chr ) const;
    bool can_accept_event( EVENTID eventid );
    BObjectImp* send_event( BObjectImp* event );
    Character* master() const;

    virtual double armor_absorb_damage( double damage );
    virtual void get_hitscript_params( double damage,
                                       UArmor** parmor,
                                       unsigned short* rawdamage );
    virtual UWeapon* intrinsic_weapon();

    void restart_script();
    void start_script();

    bool could_move( UFACING dir ) const;
    bool anchor_allows_move( UFACING dir ) const;
    bool npc_path_blocked( UFACING dir ) const;
    
    virtual BObjectImp* get_script_member( const char *membername ) const;
    virtual BObjectImp* get_script_member_id( const int id ) const; ///id test
    virtual BObjectImp* set_script_member( const char *membername, long value );
    virtual BObjectImp* set_script_member( const char *membername, const std::string& );
    virtual BObjectImp* set_script_member_id( const int id, long value );//id test
    virtual BObjectImp* set_script_member_id( const int id, const std::string& );//id test
    virtual BObjectImp* script_method( const char* methodname, Executor& ex );
    virtual BObjectImp* script_method_id( const int id, Executor& ex );
    virtual bool script_isa( unsigned isatype ) const;

    const char *classname() const;
    virtual void printOn( std::ostream& os ) const;
    virtual void printSelfOn( ostream& os ) const;
    virtual void printProperties( std::ostream& os ) const;
    virtual void printDebugProperties( std::ostream& os ) const;
    virtual void readProperties( ConfigElem& elem );
    void readNpcProperties( ConfigElem& elem );
    void loadResistances( int resistanceType, ConfigElem& elem );
    void loadDamages( int damageType, ConfigElem& elem );
    void readNewNpcAttributes( ConfigElem& elem );
    void readPropertiesForNewNPC( ConfigElem& elem );
    virtual void destroy();

    NpcTemplate::ALIGNMENT alignment() const;
	
	unsigned damaged_sound;
	bool use_adjustments; //DAVE
	unsigned short run_speed; //DAVE
protected:
	UOExecutor *ex;
	UOExecutor *give_item_ex;

    std::string template_name;
    void stop_scripts();
    friend class NPCExecutorModule;
    friend class ref_ptr<NPC>;

private: // REPUTATION SYSTEM:
    virtual void repsys_on_attack( Character* defender );
    virtual void repsys_on_damage( Character* defender );
    virtual void repsys_on_help( Character* recipient );

    virtual unsigned char hilite_color_idx( const Character* seen_by ) const;
    virtual unsigned short name_color( const Character* seen_by ) const;
	friend class RepSystem;

private:
    virtual ~NPC();
	std::string script;
    unsigned short npc_ar_;
    CharacterRef master_;
    const NpcTemplate& template_;
    struct {
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

inline Character* NPC::master() const
{
    return master_.get();
}

inline NpcTemplate::ALIGNMENT NPC::alignment() const
{
    return template_.alignment;
}

inline void npc_spoke( NPC& npc, Character *chr, const char *text, int textlen, u8 texttype )
{
	// Not happy with this comparison style, but can't think of a better alternative.
	if ( npc.serial != chr->serial )
		npc.on_pc_spoke( chr, text, texttype );
}

inline unsigned short NPC::ar() const
{
	if ( ar_ == 0 )
		return npc_ar_;
	else
		return ar_;
}

#endif
