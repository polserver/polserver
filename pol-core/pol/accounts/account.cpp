/** @file
 *
 * @par History
 * - 2005/05/25 Shinigami: added void writeto( ConfigElem& elem )
 * - 2005/05/25 Shinigami: added getnextfreeslot()
 * - 2009/08/06 MuadDib:   Removed PasswordOnlyHash support
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 */


#include "account.h"

#include <cstring>

#include "../../clib/cfgelem.h"
#include "../../clib/clib_MD5.h"
#include "../../clib/rawtypes.h"
#include "../../clib/streamsaver.h"
#include "../../plib/systemstate.h"
#include "../cmdlevel.h"
#include "../globals/uvars.h"
#include "../mobile/charactr.h"
#include "../network/client.h"
#include "../polcfg.h"
#include "../uobject.h"

namespace Pol
{
namespace Accounts
{
Account::Account( Clib::ConfigElem& elem )
    : characters_(),
      name_( elem.remove_string( "NAME" ) ),
      enabled_( true ),
      banned_( false ),
      props_( Core::CPropProfiler::Type::ACCOUNT ),
      default_cmdlevel_( 0 )
{
  // If too low, will cause the client to freeze and the console to report
  // Exception in message handler 0x91: vector
  for ( int i = 0; i < Plib::systemstate.config.character_slots; i++ )
    characters_.push_back( Core::CharacterRef( nullptr ) );

  readfrom( elem );
}

void Account::readfrom( Clib::ConfigElem& elem )
{
  if ( elem.has_prop( "Password" ) )
  {
    std::string temppass = elem.remove_string( "Password" );
    if ( Plib::systemstate.config.retain_cleartext_passwords )
    {
      password_ = temppass;
    }
    if ( !Clib::MD5_Encrypt( name_ + temppass, passwordhash_ ) )  // MD5
      elem.throw_error( "Failed to encrypt password for " + name_ );
    Plib::systemstate.accounts_txt_dirty = true;
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
  Core::CmdLevel* cmdlevel_search = Core::find_cmdlevel( cmdaccstr.c_str() );
  if ( cmdlevel_search != nullptr )
    default_cmdlevel_ = cmdlevel_search->cmdlevel;
  else
    elem.throw_error( "Didn't understand cmdlevel of '" + cmdaccstr + "'" );

  props_.clear();
  props_.readProperties( elem );
}

void Account::writeto( Clib::StreamWriter& sw ) const
{
  sw.write( "Account\n{\n" );
  sw.add( "Name", name_ );

  // dave 6/5/3 don't write cleartext unless configured to
  if ( Plib::systemstate.config.retain_cleartext_passwords && !password_.empty() )
    sw.add( "Password", password_ );

  sw.add( "PasswordHash", passwordhash_ );  // MD5

  sw.add( "Enabled", enabled_ );

  if ( !default_privs_.empty() )
  {
    sw.add( "DefaultPrivs", default_privs_.extract() );
  }
  if ( default_cmdlevel_ )
  {
    sw.add( "DefaultCmdLevel", Core::gamestate.cmdlevels[default_cmdlevel_].name.c_str() );
  }
  if ( uo_expansion_ )
  {
    sw.add( "UOExpansion", uo_expansion() );
  }
  props_.printProperties( sw );

  sw.write( "}\n\n" );
}

void Account::writeto( Clib::ConfigElem& elem ) const
{
  elem.add_prop( "Name", name_ );

  // dave 6/5/3 don't write cleartext unless configured to
  if ( Plib::systemstate.config.retain_cleartext_passwords && !password_.empty() )
    elem.add_prop( "Password", password_ );

  elem.add_prop( "PasswordHash", passwordhash_ );

  elem.add_prop( "Enabled", ( (unsigned int)( enabled_ ? 1 : 0 ) ) );
  elem.add_prop( "Banned", ( (unsigned int)( banned_ ? 1 : 0 ) ) );

  if ( !default_privs_.empty() )
  {
    elem.add_prop( "DefaultPrivs", default_privs_.extract() );
  }
  if ( default_cmdlevel_ )
  {
    elem.add_prop( "DefaultCmdLevel", Core::gamestate.cmdlevels[default_cmdlevel_].name );
  }
  if ( uo_expansion_ )
  {
    elem.add_prop( "UOExpansion", uo_expansion() );
  }
  props_.printProperties( elem );
}

Account::~Account()
{
  for ( int i = 0; i < Plib::systemstate.config.character_slots; i++ )
  {
    if ( characters_[i].get() )
    {
      if ( characters_[i]->client != nullptr && characters_[i]->client->acct == this )
        characters_[i]->client->acct = nullptr;

      characters_[i]->acct.clear();
      characters_[i].clear();
    }
  }
}

size_t Account::estimatedSize() const
{
  size_t size =
      sizeof( Account ) + name_.capacity() + password_.capacity() + passwordhash_.capacity();
  size += 3 * sizeof( Core::CharacterRef* ) + characters_.capacity() * sizeof( Core::CharacterRef );
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

/// Returns true if at least one character from this account is already logged in
bool Account::has_active_characters()
{
  for ( unsigned short i = 0; i < Plib::systemstate.config.character_slots; i++ )
    if ( characters_[i].get() && characters_[i]->client )
      return true;
  return false;
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
  case Network::TOL:
    return "TOL";
  case Network::HSA:
    return "HSA";
  case Network::SA:
    return "SA";
  case Network::KR:
    return "KR";
  case Network::ML:
    return "ML";
  case Network::SE:
    return "SE";
  case Network::AOS:
    return "AOS";
  case Network::LBR:
    return "LBR";
  case Network::T2A:
    return "T2A";
  default:
    return "";
  }
}

unsigned short Account::uo_expansion_flag() const
{
  return uo_expansion_;
}

u16 Account::convert_uo_expansion( const std::string& expansion )
{
  const auto not_found = std::string::npos;

  if ( expansion.find( "TOL" ) != not_found )
    return Network::TOL;
  else if ( expansion.find( "HSA" ) != not_found )
    return Network::HSA;
  else if ( expansion.find( "SA" ) != not_found )
    return Network::SA;
  else if ( expansion.find( "KR" ) != not_found )
    return Network::KR;
  else if ( expansion.find( "ML" ) != not_found )
    return Network::ML;
  else if ( expansion.find( "SE" ) != not_found )
    return Network::SE;
  else if ( expansion.find( "AOS" ) != not_found )
    return Network::AOS;
  else if ( expansion.find( "LBR" ) != not_found )
    return Network::LBR;
  else if ( expansion.find( "T2A" ) != not_found )
    return Network::T2A;

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
  for ( int i = 0; i < Plib::systemstate.config.character_slots; i++ )
    if ( characters_[i].get() )
      ++num;
  return num;
}

int Account::getnextfreeslot() const
{
  for ( int i = 0; i < Plib::systemstate.config.character_slots; i++ )
    if ( !characters_[i].get() )
      return ( i + 1 );
  return -1;
}
}  // namespace Accounts
}  // namespace Pol
