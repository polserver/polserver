#ifndef POL_BASE_AREA_H
#define POL_BASE_AREA_H

#include "position.h"
#include <iterator>

namespace Pol
{
namespace Core
{
class Area2dItr
{
private:
  Pos2d _v;
  u16 _xbound;
  u16 _xstart;

public:
  Area2dItr( Pos2d v, const Pos2d& v_max );

  typedef Pos2d value_type;
  typedef std::ptrdiff_t difference_type;
  typedef const Pos2d* pointer;
  typedef const Pos2d& reference;
  typedef std::input_iterator_tag iterator_category;

  reference operator*() const;
  bool operator==( const Area2dItr& other ) const;
  bool operator!=( const Area2dItr& other ) const;
  // does someone really need itr++?
  Area2dItr& operator++();
};

class Area2d
{
private:
  Pos2d _pL;
  Pos2d _pH;

public:
  Area2d() = default;
  Area2d( const Pos2d& p1, const Pos2d& p2, const Realms::Realm* realm );
  Area2d( const Pos4d& p1, const Pos4d& p2 );
  Area2d( const Area2d& other ) = default;
  Area2d( Area2d&& other ) = default;
  ~Area2d() = default;
  Area2d& operator=( const Area2d& other ) = default;
  Area2d& operator=( Area2d&& other ) = default;

  const Pos2d& posL() const;
  const Pos2d& posH() const;

  Area2dItr begin() const;
  Area2dItr end() const;

  bool contains( const Pos2d& other ) const;
  bool intersect( const Area2d& other ) const;
};


inline Area2dItr::reference Area2dItr::operator*() const
{
  return _v;
}

inline const Pos2d& Area2d::posL() const
{
  return _pL;
}
inline const Pos2d& Area2d::posH() const
{
  return _pH;
}
}  // namespace Core
}  // namespace Pol

#endif
