/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_BERROR_H
#define BSCRIPT_BERROR_H

#include <istream>
#include <string>

#include "../clib/compilerspecifics.h"
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

  virtual BObjectImp* copy() const POL_OVERRIDE;
  virtual BObjectRef OperSubscript( const BObject& obj ) POL_OVERRIDE;
  virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy ) POL_OVERRIDE;

  virtual char packtype() const POL_OVERRIDE;
  virtual const char* typetag() const POL_OVERRIDE;
  virtual const char* typeOf() const POL_OVERRIDE;
  virtual u8 typeOfInt() const POL_OVERRIDE;

  virtual bool operator==( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual bool operator<( const BObjectImp& objimp ) const POL_OVERRIDE;
  virtual bool isTrue() const POL_OVERRIDE;

  virtual ContIterator* createIterator( BObject* pIterVal ) POL_OVERRIDE;

private:
  static unsigned int creations_;
};
}
}
#endif
