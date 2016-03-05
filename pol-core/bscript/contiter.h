/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_CONTITER_H
#define BSCRIPT_CONTITER_H
namespace Pol
{
namespace Bscript
{
class ContIterator : public BObjectImp
{
public:
  ContIterator();

  virtual BObject* step();

  BObjectImp* copy( void ) const;
  size_t sizeEstimate() const;
  std::string getStringRep() const;

};

}
}
#endif
