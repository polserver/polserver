/** @file
 *
 * @par History
 * - 2009/09/05 Turley:    Added struct .? and .- as shortcut for .exists() and .erase()
 * - 2009/12/21 Turley:    ._method() call fix
 */


#ifndef BSCRIPT_BSTRUCT_H
#define BSCRIPT_BSTRUCT_H

#ifndef BSCRIPT_BOBJECT_H
#include "bobject.h"
#endif

#include <iosfwd>
#include <map>
#include <string>

#include "../clib/maputil.h"
#include "../clib/rawtypes.h"


namespace Pol::Bscript
{
class ContIterator;
class BClassInstanceRef;
class Executor;
}  // namespace Pol::Bscript


namespace Pol::Bscript
{
class BStruct : public BObjectImp
{
public:
  BStruct();
  BStruct( std::istream& is, unsigned size, BObjectType type );
  BStruct& operator=( const BStruct& ) = delete;
  static BObjectImp* unpack( std::istream& is );

  void addMember( const char* name, BObjectRef val );
  void addMember( const char* name, BObjectImp* imp );

  const BObjectImp* FindMember( const char* name );

  size_t mapcount() const;

  using Contents = std::map<std::string, BObjectRef, Clib::ci_cmp_pred>;
  const Contents& contents() const;

protected:
  explicit BStruct( const BStruct& other, BObjectType type );
  explicit BStruct( BObjectType type );

  BObjectImp* copy() const override;

  virtual char packtype() const;
  virtual const char* typetag() const;
  virtual void FormatForStringRep( std::ostream& os, const std::string& key,
                                   const BObjectRef& bvalref ) const;

  std::string getStringRep() const override;
  size_t sizeEstimate() const override;
  void packonto( std::ostream& os ) const override;
  const char* typeOf() const override;
  u8 typeOfInt() const override;

  ContIterator* createIterator( BObject* pIterVal ) override;

  BObjectRef OperSubscript( const BObject& obj ) override;
  BObjectImp* call_method( const char* methodname, Executor& ex ) override;
  BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin = false ) override;
  BObjectRef set_member( const char* membername, BObjectImp* value, bool copy ) override;
  BObjectRef get_member( const char* membername ) override;
  BObjectRef operDotPlus( const char* name ) override;
  BObjectRef operDotMinus( const char* name ) override;
  BObjectRef operDotQMark( const char* name ) override;
  BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy ) override;

  friend class BStructIterator;
  friend class BClassInstanceRef;

private:
  Contents contents_;
};
}  // namespace Pol::Bscript

#endif
