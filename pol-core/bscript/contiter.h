/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_CONTITER_H
#define BSCRIPT_CONTITER_H

#include "bobject.h"

#include <typeinfo>


namespace Pol::Bscript
{
class ContIterator : public BObjectImp
{
public:
  ContIterator();

  virtual BObject* step();

  BObjectImp* copy() const override;
  size_t sizeEstimate() const override;
  std::string getStringRep() const override;

  bool is_default() const { return typeid( *this ) == typeid( ContIterator ); }
};
}  // namespace Pol::Bscript

#endif
