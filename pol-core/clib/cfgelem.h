/** @file
 *
 * @par History
 */


#ifndef CLIB_CFGELEM_H
#define CLIB_CFGELEM_H
#include "maputil.h"

#include <map>
#include <vector>


namespace Pol::Clib
{
class ConfigProperty
{
public:
  ConfigProperty( std::string name, std::string value );
  ~ConfigProperty() = default;

protected:
  std::string name_;
  std::string value_;
  friend class ConfigElem;
};

class ConfigSource;

class ConfigElemBase
{
public:
  virtual ~ConfigElemBase() = default;
  bool type_is( const char* name ) const;
  const char* type() const;
  const char* rest() const;
  virtual size_t estimateSize() const;

protected:
  ConfigElemBase();

protected:
  std::string type_;
  std::string rest_;

  const ConfigSource* _source;
};

class ConfigElem : public ConfigElemBase
{
public:
  ConfigElem();
  ~ConfigElem() override;
  size_t estimateSize() const override;
  friend class ConfigFile;

  bool has_prop( const char* propname ) const;

  std::string remove_string( const char* propname );
  std::string remove_string( const char* propname, const char* dflt );

  unsigned short remove_ushort( const char* propname );
  unsigned short remove_ushort( const char* propname, unsigned short dflt );

  int remove_int( const char* propname );
  int remove_int( const char* propname, int dflt );

  unsigned remove_unsigned( const char* propname );
  unsigned remove_unsigned( const char* propname, int dflt );

  unsigned int remove_ulong( const char* propname );
  unsigned int remove_ulong( const char* propname, unsigned int dflt );

  bool remove_bool( const char* propname );
  bool remove_bool( const char* propname, bool dflt );

  float remove_float( const char* propname, float dflt );
  double remove_double( const char* propname, double dflt );

  void clear_prop( const char* propname );

  bool remove_first_prop( std::string* propname, std::string* value );
  bool remove_prop( const char* propname, std::string* value );
  bool remove_prop( const char* propname, unsigned int* plong );
  bool remove_prop( const char* propname, unsigned short* pushort );

  bool read_prop( const char* propname, std::string* value ) const;

  // get_prop calls: don't remove, and throw if not found.
  void get_prop( const char* propname, unsigned int* plong ) const;

  std::string read_string( const char* propname ) const;
  std::string read_string( const char* propname, const char* dflt ) const;


  void add_prop( std::string propname, std::string propval );
  void add_prop( std::string propname, unsigned int lval );
  void add_prop( std::string propname, unsigned short sval );
  void add_prop( std::string propname, short sval );

  [[noreturn]] void throw_error( const std::string& errmsg ) const;
  [[noreturn]] void throw_prop_not_found( const std::string& propname ) const;
  void warn( const std::string& errmsg ) const;
  void warn_with_line( const std::string& errmsg ) const;

  void set_rest( const char* newrest );
  void set_type( const char* newtype );
  void set_source( const ConfigElem& elem );
  void set_source( const ConfigSource* source );

protected:
  [[noreturn]] void prop_not_found( const char* propname ) const;
  using Props = std::multimap<std::string, std::string, ci_cmp_pred>;
  Props properties;
};

}  // namespace Pol::Clib

#endif
