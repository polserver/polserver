#pragma once

namespace Pol::Core
{
class UObject;
}
namespace Pol::Items
{
class Item;
}
namespace Pol::Mobile
{
class Character;

// wrapper for Mobiles and attackable Items
// extend it with methods so that not everywhere if (mobile()).. has to be used
class Attackable
{
public:
  Attackable() = default;
  Attackable( Character* chr );
  Attackable( Items::Item* item );

  explicit operator bool() const { return _opp != nullptr; };
  void clear() { _opp = nullptr; };

  Core::UObject* object() const { return _opp; };
  Character* mobile() const;
  Items::Item* item() const;

  // std::less support for std::set
  bool operator<( const Attackable& o ) const { return _opp < o._opp; };

private:
  Core::UObject* _opp = nullptr;
};
}  // namespace Pol::Mobile
