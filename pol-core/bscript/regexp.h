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

  // const boost::regex& regex() const { return regex_; }
  // boost::match_flag_type flags() const { return match_flags_; }

  // All return imps because they can _at least_ return a BError
  BObjectImp* find( const String* str, int start ) const;
  BObjectImp* match( const String* str ) const;
  BObjectImp* replace( const String* str, const String* replacement ) const;
  BObjectImp* split( const String* str, size_t limit ) const;

  // cannot be const because we create a BObjectRef(this) to continuation lambda
  BObjectImp* replace( Executor& ex, const String* str, BFunctionRef* replacement_callback );

protected:
  virtual BObjectImp* copy() const override;

  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override;

  virtual bool operator<( const BObjectImp& objimp ) const override;
  virtual bool isTrue() const override;

private:
  using RegexT = std::variant<boost::regex, boost::wregex>;

  explicit BRegExp( RegexT regex, boost::match_flag_type match_flags );
  BRegExp( const BRegExp& i );

  RegexT regex_;
  boost::match_flag_type match_flags_;
};
}  // namespace Pol::Bscript
