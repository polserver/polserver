/*
History
=======
2005/05/25 Shinigami: added void writeto( ConfigElem& elem )
2005/05/25 Shinigami: added getnextfreeslot()
2009/08/06 MuadDib:   Removed PasswordOnlyHash support
2009/09/03 MuadDib:   Relocation of account related cpp/h

Notes
=======

*/

#include "../../clib/stl_inc.h"
#include "../../clib/cfgelem.h"
#include "../../clib/MD5.h"

#include "account.h"
#include "accounts.h"
#include "../mobile/charactr.h"
#include "../network/client.h"
#include "../cmdlevel.h"
#include "../polcfg.h"
#include <cstring>

Account::Account( ConfigElem& elem ) :
    active_character(NULL),
    characters_(),
    name_(elem.remove_string( "NAME" )),
    enabled_(true),
    banned_(false),
    default_cmdlevel_(0)
{
	// If too low, will cause the client to freeze and the console to report 
	// Exception in message handler 0x91: vector
	for( int i = 0; i < config.character_slots; i++ )
        characters_.push_back( CharacterRef(NULL) );
    
    readfrom( elem );
}

void Account::readfrom( ConfigElem& elem )
{
	if( elem.has_prop("Password") )
	{
		string temppass;
		temppass = elem.remove_string( "Password" );
		if(config.retain_cleartext_passwords)
		{
			password_ = temppass;
		}
		if(!MD5_Encrypt(name_+temppass,passwordhash_))	//MD5
			elem.throw_error("Failed to encrypt password for " + name_);
		accounts_txt_dirty = true;
	}
	else if( elem.has_prop("PasswordHash") )
	{
		passwordhash_ = elem.remove_string( "PasswordHash" );
	}
	else
		elem.throw_error("Failed password reads for account " + name_);

    enabled_ = elem.remove_bool( "ENABLED", true );
    banned_ = elem.remove_bool( "BANNED", false );
	uo_expansion_ = convert_uo_expansion(elem.remove_string( "UOExpansion", "T2A" ));

    default_privs_.readfrom( elem.remove_string( "DefaultPrivs", "" ) );

    string cmdaccstr = elem.remove_string( "DefaultCmdLevel", "player" );
    CmdLevel* cmdlevel_search = find_cmdlevel( cmdaccstr.c_str() );
    if (cmdlevel_search == NULL)
            elem.throw_error("Didn't understand cmdlevel of '" + cmdaccstr + "'");
    default_cmdlevel_ = cmdlevel_search->cmdlevel;

    props_.clear();
    props_.readProperties( elem );
}

void Account::writeto( std::ostream& os )
{
    os << "Account" << std::endl
		<< "{" << std::endl
       << "\tName\t"     << name_     << std::endl;

	//dave 6/5/3 don't write cleartext unless configured to
	if(config.retain_cleartext_passwords && !password_.empty())
		os << "\tPassword\t" << password_ << std::endl;

	os << "\tPasswordHash\t" << passwordhash_ << std::endl;  //MD5

    os << "\tEnabled\t"  << enabled_  << std::endl
       << "\tBanned\t"   << banned_   << std::endl;

    if (!default_privs_.empty())
    {
        os << "\tDefaultPrivs\t" << default_privs_.extract() << pf_endl;
    }
    if (default_cmdlevel_)
    {
        os << "\tDefaultCmdLevel\t" << cmdlevels2[default_cmdlevel_].name.c_str() << pf_endl;
    }
	if (uo_expansion_)
	{
		os << "\tUOExpansion\t" << uo_expansion() << pf_endl;
	}
    props_.printProperties( os );
    
    os << "}" << endl
       << endl;
}

void Account::writeto( ConfigElem& elem ) const
{
    elem.add_prop( "Name", name_.c_str() );	

	//dave 6/5/3 don't write cleartext unless configured to
	if(config.retain_cleartext_passwords && !password_.empty())
        elem.add_prop( "Password", password_.c_str() );

    elem.add_prop( "PasswordHash", passwordhash_.c_str() );	

    elem.add_prop( "Enabled", ((unsigned int)(enabled_?1:0)) );
    elem.add_prop( "Banned", ((unsigned int)(banned_?1:0)) );
       
    if (!default_privs_.empty())
    {
        elem.add_prop( "DefaultPrivs", default_privs_.extract().c_str() );	
    }
    if (default_cmdlevel_)
    {
        elem.add_prop( "DefaultCmdLevel", cmdlevels2[default_cmdlevel_].name.c_str() );	
    }
	if (uo_expansion_)
	{
        elem.add_prop( "UOExpansion", uo_expansion().c_str() );	
	}
    props_.printProperties( elem );
}

Account::~Account()
{
    if (active_character != NULL)
    {
        if ((active_character->client != NULL) &&
            (active_character->client->acct == this))
        {
            active_character->client->acct = NULL;
        }

        if (active_character->acct == this)
        {
            active_character->acct.clear();
        }

        active_character = NULL;
    }
    
	for( int i = 0; i < config.character_slots; i++ )
    {
        if (characters_[i].get())
        {
            characters_[i]->acct.clear();
            characters_[i].clear();
        }
    }
}

Character* Account::get_character( int index )
{
    return characters_.at(index).get();
}

void Account::set_character( int index, Character* chr )
{
    characters_.at(index).set( chr );
}

void Account::clear_character( int index )
{
    characters_.at(index).clear();
}

const char* Account::name() const
{
    return name_.c_str();
}

const string Account::password() const
{
    return password_;
}

const string Account::passwordhash() const
{
    return passwordhash_;
}

const string Account::uo_expansion() const
{
	switch (uo_expansion_)
	{
	case SA: return "SA";
	case KR: return "KR";
	case ML: return "ML";
	case SE: return "SE";
	case AOS: return "AOS";
	case LBR: return "LBR";
	case T2A: return "T2A";
	default: return "";
	}
}

const unsigned short Account::uo_expansion_flag() const
{
	return uo_expansion_;
}

u16 Account::convert_uo_expansion(const std::string& expansion) const
{
	if(expansion.find("SA") != string::npos)
		return SA;
	else if(expansion.find("KR") != string::npos)
		return KR;
	else if(expansion.find("ML") != string::npos)
		return ML;
	else if(expansion.find("SE") != string::npos)
		return SE;
	else if(expansion.find("AOS") != string::npos)
		return AOS;
	else if(expansion.find("LBR") != string::npos)
		return LBR;
	else if(expansion.find("T2A") != string::npos)
		return T2A;
	else
		return 0;
}



bool Account::enabled() const
{
    return enabled_;
}

bool Account::banned() const
{
    return banned_;
}

std::string Account::default_privlist() const
{
    return default_privs_.extract();
}

unsigned char Account::default_cmdlevel() const
{
    return default_cmdlevel_;
}

int Account::numchars() const
{
	int num = 0;
	for( int i = 0; i < config.character_slots; i++ )
		if ( characters_[i].get() )
			++num;
	return num;
}

int Account::getnextfreeslot() const
{
	for( int i = 0; i < config.character_slots; i++ )
		if ( !characters_[i].get() )
            return (i+1);
	return -1;
}
