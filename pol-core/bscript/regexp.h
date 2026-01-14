#pragma once

#include <boost/regex.hpp>
#include <variant>

#include "bobject.h"

namespace Pol::Bscript
{
class BRegExp final : public BObjectImp
{
public:
  // Can return BError if pattern or flags are invalid
  static BObjectImp* create( const std::string& pattern, const std::string& flags );

  // All return imps because they can return a BError
  BObjectImp* find( const String* str, int start ) const;
  BObjectImp* match( const String* str ) const;
  BObjectImp* replace( const String* str, const String* replacement ) const;
  BObjectImp* split( const String* str, size_t limit ) const;

  // cannot be const because we create a BObjectRef(this) to continuation lambda
  BObjectImp* replace( Executor& ex, const String* str, BFunctionRef* replacement_callback );

protected:
  BObjectImp* copy() const override;

  const char* typeOf() const override;
  u8 typeOfInt() const override;
  std::string getStringRep() const override;
  size_t sizeEstimate() const override;

  bool operator<( const BObjectImp& objimp ) const override;
  bool isTrue() const override;

private:
  using RegexT = std::variant<boost::regex, boost::wregex>;

  explicit BRegExp( RegexT regex, boost::match_flag_type match_flags );
  BRegExp( const BRegExp& i );

  RegexT regex_;
  boost::match_flag_type match_flags_;
};
}  // namespace Pol::Bscript
