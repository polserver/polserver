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
  bool global() const { return global_; }
  bool multiline() const { return multiline_; }

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
  bool global_;
  bool multiline_;
};
}  // namespace Pol::Bscript
