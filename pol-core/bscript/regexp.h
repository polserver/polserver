#pragma once

#include <boost/regex.hpp>

#include "bobject.h"

namespace Pol::Bscript
{
class BRegExp final : public BObjectImp
{
public:
  // Can return BError if pattern or flags are invalid
  static BObjectImp* create( const std::string& pattern, const std::string& flags );

  const boost::regex& regex() const { return regex_; }
  boost::match_flag_type flags() const { return match_flags_; }

protected:
  virtual BObjectImp* copy() const override;

  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override;

  virtual bool operator<( const BObjectImp& objimp ) const override;
  virtual bool isTrue() const override;

private:
  explicit BRegExp( boost::regex regex, boost::match_flag_type match_flags );
  BRegExp( const BRegExp& i );

  boost::regex regex_;
  boost::match_flag_type match_flags_;
};
}  // namespace Pol::Bscript
