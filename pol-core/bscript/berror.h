/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_BERROR_H
#define BSCRIPT_BERROR_H

#include <istream>
#include <string>

#include "../clib/rawtypes.h"
#include "bobject.h"
#include "bstruct.h"

namespace Pol
{
namespace Bscript
{
class ContIterator;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Bscript
{
class BError final : public BStruct
{
public:
  BError();
  explicit BError( const char* errortext );
  explicit BError( const std::string& errortext );

  static BObjectImp* unpack( std::istream& is );

  static unsigned int creations();

protected:
  BError( const BError& i );
  BError( std::istream& is, unsigned size );

  virtual BObjectImp* copy() const override;
  virtual BObjectRef OperSubscript( const BObject& obj ) override;
  virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy ) override;

  virtual char packtype() const override;
  virtual const char* typetag() const override;
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;

  virtual bool operator==( const BObjectImp& objimp ) const override;
  virtual bool operator<( const BObjectImp& objimp ) const override;
  virtual bool isTrue() const override;

  virtual ContIterator* createIterator( BObject* pIterVal ) override;

private:
  static unsigned int creations_;
};
}
}
#endif
