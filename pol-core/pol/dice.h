/** @file
 *
 * @par History
 * - 2005/07/25 Shinigami: added min_value and max_value to populate Weapon-Descriptor
 */


#ifndef DICE_H
#define DICE_H

#include <string>
namespace Pol
{
namespace Core
{
class Dice
{
public:
  Dice();
  bool load( const char* dice, std::string* errormsg );
  unsigned short roll( void ) const;
  int roll_with_negatives() const;
  void die_string( std::string& str ) const;
  unsigned short min_value( void ) const;
  unsigned short max_value( void ) const;

private:
  unsigned die_count;
  unsigned die_type;
  int plus_damage;
};
}  // namespace Core
}  // namespace Pol
#endif
