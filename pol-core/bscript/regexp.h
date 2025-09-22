#pragma once

#include <regex>

#include "bobject.h"

namespace Pol::Bscript
{
class BRegExp final : public BObjectImp
{
public:
  explicit BRegExp( const std::string& expr, std::regex_constants::syntax_option_type flags,
                    bool global );

  const std::regex& regex() const { return regex_; }
  bool global() const { return global_; }

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
  std::regex regex_;
  bool global_;
};
}  // namespace Pol::Bscript
