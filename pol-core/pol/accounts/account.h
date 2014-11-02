/*
History
=======
2005/05/25 Shinigami: added void writeto( ConfigElem& elem )
2005/05/25 Shinigami: added getnextfreeslot()
2009/08/06 MuadDib:   Removed PasswordOnlyHash support

Notes
=======

*/

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "../ucfg.h"
#include "../proplist.h"
#include "../reftypes.h"

#include "../../clib/strset.h"
#include "../../clib/refptr.h"

#include <vector>
#include <string>

namespace Pol {
  namespace Mobile {
	class Character;
  }
  namespace Network {
	class Client;
  }
  namespace Accounts {

	class Account : public ref_counted
	{
	public:
	  explicit Account( Clib::ConfigElem& elem );
	  ~Account();

      size_t estimatedSize() const;

	  const char *name() const;
	  const std::string password() const;
	  const std::string passwordhash() const;
	  const std::string uo_expansion() const;
	  const unsigned short uo_expansion_flag() const;
	  static unsigned short convert_uo_expansion( const std::string& expansion );
	  bool enabled() const;
	  bool banned() const;

	  int numchars() const;
	  int getnextfreeslot() const;

	  Mobile::Character* active_character;

	  Mobile::Character* get_character( int index );
	  void set_character( int index, Mobile::Character* chr );
	  void clear_character( int index );

	  void readfrom( Clib::ConfigElem& elem );
	  void writeto( Clib::StreamWriter& sw ) const;
	  void writeto( Clib::ConfigElem& elem ) const;

	  std::string default_privlist() const;
	  unsigned char default_cmdlevel() const;

	  void set_password( std::string newpass ) { password_ = newpass; };
	  void set_passwordhash( std::string newpass ) { passwordhash_ = newpass; };

	  friend class AccountObjImp;

	private:
	  std::vector<Core::CharacterRef> characters_;
	  std::string name_;
	  std::string password_;
	  std::string passwordhash_;
	  unsigned short uo_expansion_;
	  bool enabled_;
	  bool banned_;
	  Core::PropertyList props_;
	  Clib::StringSet default_privs_;
	  unsigned char default_cmdlevel_;

	  Clib::StringSet options_;
	};

	Account *find_account( const char *acctname );
  }
}
#endif
