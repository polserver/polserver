/** @file
 *
 * @par History
 */


#ifndef CLIB_CFGELEM_H
#define CLIB_CFGELEM_H
#include "maputil.h"

#include <map>
#include <vector>

namespace Pol
{
namespace Clib
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
  friend class VectorConfigElem;
};

class ConfigSource;

class ConfigElemBase
{
public:
  virtual ~ConfigElemBase(){};
  bool type_is( const char* name ) const;
  const char* type() const;
  const char* rest() const;
  virtual size_t estimateSize() const;

  [[noreturn]] void throw_error( const std::string& errmsg ) const;

protected:
  ConfigElemBase();
  unsigned short string_to_u16( const char* prop, const std::string& str );

protected:
  std::string type_;
  std::string rest_;

  const ConfigSource* _source;
};

class ConfigElem : public ConfigElemBase
{
public:
  ConfigElem();
  virtual ~ConfigElem();
  virtual size_t estimateSize() const override;
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
  unsigned short read_ushort( const char* propname, unsigned short dflt );
  unsigned short read_ushort( const char* propname );

  int read_int( const char* propname, int dflt );
  int read_int( const char* propname );


  void add_prop( std::string propname, std::string propval );
  void add_prop( std::string propname, unsigned int lval );
  void add_prop( std::string propname, unsigned short sval );
  void add_prop( std::string propname, short sval );

  void warn( const std::string& errmsg ) const;
  void warn_with_line( const std::string& errmsg ) const;

  void set_rest( const char* newrest );
  void set_type( const char* newtype );
  void set_source( const ConfigElem& elem );
  void set_source( const ConfigSource* source );

protected:
  [[noreturn]] void prop_not_found( const char* propname ) const;
  typedef std::multimap<std::string, std::string, ci_cmp_pred> Props;
  Props properties;
};

class VectorConfigElem : public ConfigElemBase
{
public:
  VectorConfigElem();
  virtual ~VectorConfigElem();

  friend class ConfigFile;
  bool type_is( const char* name ) const;
  const char* type() const;
  const char* rest() const;

  bool has_prop( const char* propname ) const;

  std::string remove_string( const char* propname );
  std::string remove_string( const char* propname, const char* dflt );

  unsigned short remove_ushort( const char* propname );
  unsigned short remove_ushort( const char* propname, unsigned short dflt );

  int remove_int( const char* propname );
  int remove_int( const char* propname, int dflt );

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
  std::string read_string( const char* propname ) const;
  std::string read_string( const char* propname, const char* dflt ) const;


  void add_prop( std::string propname, std::string propval );
  void add_prop( std::string propname, unsigned int lval );
  void add_prop( std::string propname, unsigned short sval );

  void warn( const std::string& errmsg ) const;

  void set_rest( const char* newrest );
  void set_source( const ConfigElem& elem );

protected:
  [[noreturn]] void prop_not_found( const char* propname ) const;
  std::string type_;
  std::string rest_;

  typedef std::vector<ConfigProperty*> Props;
  Props properties;

  ConfigSource* source_;
};
}  // namespace Clib
}  // namespace Pol
#endif
