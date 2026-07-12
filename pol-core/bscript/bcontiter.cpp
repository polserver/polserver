#include "bcontiter.h"

#include <typeinfo>

namespace Pol::Bscript
{
ContIterator::ContIterator() : BObjectImp( BObjectImp::OTUnknown ) {}
BObject* ContIterator::step()
{
  return nullptr;
}
BObjectImp* ContIterator::copy() const
{
  return nullptr;
}
size_t ContIterator::sizeEstimate() const
{
  return sizeof( ContIterator );
}
std::string ContIterator::getStringRep() const
{
  return "<iterator>";
}
bool ContIterator::is_default() const
{
  return typeid( *this ) == typeid( ContIterator );
}
}  // namespace Pol::Bscript
