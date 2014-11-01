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

#include "account.h"
#include "accounts.h"

#include "../../clib/cfgelem.h"
#include "../../clib/MD5.h"
#include "../../clib/streamsaver.h"

#include "../mobile/charactr.h"
#include "../network/client.h"
#include "../cmdlevel.h"
#include "../polcfg.h"

#include <cstring>

namespace Pol {
  namespace Accounts {
	Account::Account( Clib::ConfigElem& elem ) :
	  active_character( NULL ),
	  characters_(),
	  name_( elem.remove_string( "NAME" ) ),
	  enabled_( true ),
	  banned_( false ),
	  default_cmdlevel_( 0 )
	{
	  // If too low, will cause the client to freeze and the console to report 
	  // Exception in message handler 0x91: vector
	  for ( int i = 0; i < Core::config.character_slots; i++ )
		characters_.push_back( Core::CharacterRef( NULL ) );

	  readfrom( elem );
	}

	void Account::readfrom( Clib::ConfigElem& elem )
	{
	  if ( elem.has_prop( "Password" ) )
	  {
        std::string temppass = elem.remove_string("Password");
		if ( Core::config.retain_cleartext_passwords )
		{
		  password_ = temppass;
		}
		if ( !Clib::MD5_Encrypt( name_ + temppass, passwordhash_ ) )	//MD5
		  elem.throw_error( "Failed to encrypt password for " + name_ );
		accounts_txt_dirty = true;
	  }
	  else if ( elem.has_prop( "PasswordHash" ) )
	  {
		passwordhash_ = elem.remove_string( "PasswordHash" );
	  }
	  else
		elem.throw_error( "Failed password reads for account " + name_ );

	  enabled_ = elem.remove_bool( "ENABLED", true );
	  banned_ = elem.remove_bool( "BANNED", false );
	  uo_expansion_ = convert_uo_expansion( elem.remove_string( "UOExpansion", "T2A" ) );

	  default_privs_.readfrom( elem.remove_string( "DefaultPrivs", "" ) );

	  std::string cmdaccstr = elem.remove_string( "DefaultCmdLevel", "player" );
	  Core::CmdLevel* cmdlevel_search = Core::find_cmdlevel( cmdaccstr.c_str( ) );
	  if ( cmdlevel_search != NULL )
		default_cmdlevel_ = cmdlevel_search->cmdlevel;
	  else
		elem.throw_error( "Didn't understand cmdlevel of '" + cmdaccstr + "'" );

	  props_.clear();
	  props_.readProperties( elem );
	}

	void Account::writeto( Clib::StreamWriter& sw ) const
	{
	  sw() << "Account" << pf_endl
		<< "{" << pf_endl
		<< "\tName\t" << name_ << pf_endl;

	  //dave 6/5/3 don't write cleartext unless configured to
	  if ( Core::config.retain_cleartext_passwords && !password_.empty( ) )
		sw() << "\tPassword\t" << password_ << pf_endl;

	  sw() << "\tPasswordHash\t" << passwordhash_ << pf_endl;  //MD5

	  sw() << "\tEnabled\t" << enabled_ << pf_endl
		<< "\tBanned\t" << banned_ << pf_endl;

	  if ( !default_privs_.empty() )
	  {
		sw() << "\tDefaultPrivs\t" << default_privs_.extract() << pf_endl;
	  }
	  if ( default_cmdlevel_ )
	  {
		sw( ) << "\tDefaultCmdLevel\t" << Core::cmdlevels2[default_cmdlevel_].name.c_str( ) << pf_endl;
	  }
	  if ( uo_expansion_ )
	  {
		sw() << "\tUOExpansion\t" << uo_expansion() << pf_endl;
	  }
	  props_.printProperties( sw );

	  sw() << "}" << pf_endl
		<< pf_endl;
	  //sw.flush();
	}

	void Account::writeto( Clib::ConfigElem& elem ) const
	{
	  elem.add_prop( "Name", name_.c_str() );

	  //dave 6/5/3 don't write cleartext unless configured to
	  if ( Core::config.retain_cleartext_passwords && !password_.empty( ) )
		elem.add_prop( "Password", password_.c_str() );

	  elem.add_prop( "PasswordHash", passwordhash_.c_str() );

	  elem.add_prop( "Enabled", ( (unsigned int)( enabled_ ? 1 : 0 ) ) );
	  elem.add_prop( "Banned", ( (unsigned int)( banned_ ? 1 : 0 ) ) );

	  if ( !default_privs_.empty() )
	  {
		elem.add_prop( "DefaultPrivs", default_privs_.extract().c_str() );
	  }
	  if ( default_cmdlevel_ )
	  {
		elem.add_prop( "DefaultCmdLevel", Core::cmdlevels2[default_cmdlevel_].name.c_str( ) );
	  }
	  if ( uo_expansion_ )
	  {
		elem.add_prop( "UOExpansion", uo_expansion().c_str() );
	  }
	  props_.printProperties( elem );
	}

	Account::~Account()
	{
	  if ( active_character != NULL )
	  {
		if ( ( active_character->client != NULL ) &&
			 ( active_character->client->acct == this ) )
		{
		  active_character->client->acct = NULL;
		}

		if ( active_character->acct == this )
		{
		  active_character->acct.clear();
		}

		active_character = NULL;
	  }

	  for ( int i = 0; i < Core::config.character_slots; i++ )
	  {
		if ( characters_[i].get() )
		{
		  characters_[i]->acct.clear();
		  characters_[i].clear();
		}
	  }
	}

    size_t Account::estimatedSize() const
    {
      size_t size = sizeof( Account )
        + name_.capacity()+ password_.capacity()+passwordhash_.capacity();
      size += 3 * sizeof( Core::CharacterRef* ) + characters_.capacity( ) * sizeof( Core::CharacterRef );
      size += props_.estimatedSize();
      size += default_privs_.estimatedSize();
      size += options_.estimatedSize();
      return size;
    }

	Mobile::Character* Account::get_character( int index )
	{
	  return characters_.at( index ).get();
	}

	void Account::set_character( int index, Mobile::Character* chr )
	{
	  characters_.at( index ).set( chr );
	}

	void Account::clear_character( int index )
	{
	  characters_.at( index ).clear();
	}

	const char* Account::name() const
	{
	  return name_.c_str();
	}

	const std::string Account::password() const
	{
	  return password_;
	}

	const std::string Account::passwordhash() const
	{
	  return passwordhash_;
	}

	const std::string Account::uo_expansion() const
	{
	  switch ( uo_expansion_ )
	  {
		case Network::HSA: return "HSA";
		case Network::SA: return "SA";
		case Network::KR: return "KR";
		case Network::ML: return "ML";
		case Network::SE: return "SE";
		case Network::AOS: return "AOS";
		case Network::LBR: return "LBR";
		case Network::T2A: return "T2A";
		default: return "";
	  }
	}

	const unsigned short Account::uo_expansion_flag() const
	{
	  return uo_expansion_;
	}

	u16 Account::convert_uo_expansion( const std::string& expansion )
    {
        const auto not_found = std::string::npos;

	  if ( expansion.find( "HSA" ) != not_found )
		return Network::HSA;
      else if (expansion.find("SA") != not_found)
		return Network::SA;
      else if (expansion.find("KR") != not_found)
		return Network::KR;
      else if (expansion.find("ML") != not_found)
		return Network::ML;
      else if (expansion.find("SE") != not_found)
		return Network::SE;
      else if (expansion.find("AOS") != not_found)
		return Network::AOS;
      else if (expansion.find("LBR") != not_found)
		return Network::LBR;
      else if (expansion.find("T2A") != not_found)
		return Network::T2A;
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
	  for ( int i = 0; i < Core::config.character_slots; i++ )
	  if ( characters_[i].get() )
		++num;
	  return num;
	}

	int Account::getnextfreeslot() const
	{
	  for ( int i = 0; i < Core::config.character_slots; i++ )
	  if ( !characters_[i].get() )
		return ( i + 1 );
	  return -1;
	}
  }
}