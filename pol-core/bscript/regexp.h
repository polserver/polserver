#pragma once

#include <boost/regex.hpp>

#include "bobject.h"

namespace Pol::Bscript
{
class BRegExp final : public BObjectImp
{
public:
  explicit BRegExp( const std::string& expr, boost::regex_constants::syntax_option_type flags,
                    bool global, bool multiline );

  const boost::regex& regex() const { return regex_; }
  boost::match_flag_type flags() const { return match_flags_; }

protected:
  BRegExp( const BRegExp& i );

  virtual BObjectImp* copy() const override;

  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override;

  virtual bool operator<( const BObjectImp& objimp ) const override;
  virtual bool isTrue() const override;

private:
  boost::regex regex_;
  boost::match_flag_type match_flags_;
};
}  // namespace Pol::Bscript
