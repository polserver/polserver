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


namespace Pol::Bscript
{
class ContIterator;
}  // namespace Pol::Bscript


namespace Pol::Bscript
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

  BObjectImp* copy() const override;
  BObjectRef OperSubscript( const BObject& obj ) override;
  BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy ) override;

  char packtype() const override;
  const char* typetag() const override;
  const char* typeOf() const override;
  u8 typeOfInt() const override;

  bool operator==( const BObjectImp& objimp ) const override;
  bool operator<( const BObjectImp& objimp ) const override;
  bool isTrue() const override;

  ContIterator* createIterator( BObject* pIterVal ) override;

private:
  static unsigned int creations_;
};
}  // namespace Pol::Bscript

#endif
