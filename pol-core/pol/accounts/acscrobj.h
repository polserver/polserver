/** @file
 *
 * @par History
 * - 2009/12/21 Turley:    ._method() call fix
 */


#ifndef ACSCROBJ_H
#define ACSCROBJ_H

#include "../../clib/rawtypes.h"
#include "../polobject.h"
#include "../reftypes.h"

namespace Pol
{
namespace Accounts
{
class Account;
}  // namespace Accounts
namespace Bscript
{
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Accounts
{
class AccountPtrHolder
{
public:
  explicit AccountPtrHolder( Core::AccountRef i_acct ) : acct( i_acct ) {}
  Account* operator->() { return acct.get(); }
  const Account* operator->() const { return acct.get(); }
  Account* Ptr() { return acct.get(); }

private:
  Core::AccountRef acct;
};

extern Bscript::BApplicObjType accountobjimp_type;
class AccountObjImp final : public Core::PolApplicObj<AccountPtrHolder>
{
  typedef Core::PolApplicObj<AccountPtrHolder> base;

public:
  explicit AccountObjImp( const AccountPtrHolder& other )
      : PolApplicObj<AccountPtrHolder>( &accountobjimp_type, other )
  {
  }
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual Bscript::BObjectImp* call_polmethod( const char* methodname,
                                            Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                               bool forcebuiltin = false ) override;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  // id test
};
}  // namespace Accounts
}  // namespace Pol
#endif
