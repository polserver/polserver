/** @file
 *
 * @par History
 * - 2005/05/25 Shinigami: added void writeto( ConfigElem& elem )
 * - 2005/05/25 Shinigami: added getnextfreeslot()
 * - 2009/08/06 MuadDib:   Removed PasswordOnlyHash support
 */


#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <vector>

#include "../../clib/refptr.h"
#include "../../clib/strset.h"
#include "../../plib/uoexpansion.h"
#include "../proplist.h"
#include "../reftypes.h"

namespace Pol
{
namespace Mobile
{
class Character;
}
namespace Accounts
{
class Account : public ref_counted
{
public:
  explicit Account( Clib::ConfigElem& elem );
  ~Account() override;

  size_t estimatedSize() const;

  const char* name() const;
  const std::string password() const;
  const std::string passwordhash() const;
  const Plib::AccountExpansion& expansion() { return expansion_; };
  bool enabled() const;
  bool banned() const;

  int numchars() const;
  int getnextfreeslot() const;

  Mobile::Character* get_character( int index );
  void set_character( int index, Mobile::Character* chr );
  void clear_character( int index );

  bool has_active_characters();

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
  bool enabled_;
  bool banned_;
  Core::PropertyList props_;
  Clib::StringSet default_privs_;
  unsigned char default_cmdlevel_;

  Clib::StringSet options_;
  Plib::AccountExpansion expansion_;
};
}  // namespace Accounts
}  // namespace Pol
#endif
