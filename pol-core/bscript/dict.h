/** @file
 *
 * @par History
 * - 2009/12/21 Turley:    ._method() call fix
 */


#ifndef BSCRIPT_DICT_H
#define BSCRIPT_DICT_H

#ifndef BSCRIPT_BOBJECT_H
#include "bobject.h"
#endif

#include <iosfwd>
#include <map>
#include <string>

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Bscript
{
class ContIterator;
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Bscript
{
class BDictionary final : public BObjectImp
{
public:
  BDictionary();

  static BObjectImp* unpack( std::istream& is );

  void addMember( const char* name, BObjectRef val );
  void addMember( const char* name, BObjectImp* imp );
  void addMember( BObjectImp* key, BObjectImp* val );
  size_t mapcount() const;

  typedef std::map<BObject, BObjectRef> Contents;
  const Contents& contents() const;

protected:
  BDictionary( std::istream& is, unsigned size, BObjectType type = OTDictionary );
  BDictionary( const BDictionary&, BObjectType type = OTDictionary );

  virtual BObjectImp* copy() const override;
  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override;
  virtual void packonto( std::ostream& os ) const override;
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;

  virtual ContIterator* createIterator( BObject* pIterVal ) override;

  char packtype() const;
  const char* typetag() const;
  void FormatForStringRep( std::ostream& os, const BObject& bkeyobj,
                           const BObjectRef& bvalref ) const;

  virtual BObjectRef OperSubscript( const BObject& obj ) override;
  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) override;
  virtual BObjectImp* call_method_id( const int id, Executor& ex,
                                      bool forcebuiltin = false ) override;
  virtual BObjectRef set_member( const char* membername, BObjectImp* value, bool copy ) override;
  virtual BObjectRef get_member( const char* membername ) override;
  virtual BObjectRef operDotPlus( const char* name ) override;
  virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy ) override;

  friend class BDictionaryIterator;

protected:
  explicit BDictionary( BObjectType type );

private:
  Contents contents_;

  // not implemented:
  BDictionary& operator=( const BDictionary& );
};
}  // namespace Bscript
}  // namespace Pol
#endif
