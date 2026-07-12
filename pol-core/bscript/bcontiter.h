/** @file
 *
 * @par History
 */

#pragma once

#include "bobjectimp.h"

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

  bool is_default() const;
};
}  // namespace Pol::Bscript
