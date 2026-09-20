/** @file
 *
 * @par History
 */


#ifndef CLIB_CFGELEM_H
#define CLIB_CFGELEM_H
#include "clib/maputil.h"
#include "clib/rawtypes.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace Pol::Clib
{
/// A property name plus its hash. Implicit constructors so a call site can pass a plain literal
/// and get the hash folded into a constant; a name built at runtime works, it just pays to hash.
class PropKey
{
public:
  constexpr PropKey( const char* name ) : name_( name ), hash_( ci_hash( name_ ) ) {}
  constexpr PropKey( std::string_view name ) : name_( name ), hash_( ci_hash( name_ ) ) {}
  PropKey( const std::string& name ) : name_( name ), hash_( ci_hash( name_ ) ) {}

  constexpr std::string_view name() const { return name_; }
  constexpr u32 hash() const { return hash_; }

  /// Case-insensitive, like the hash; the hash compare ahead of it is only a cheap rejection.
  friend bool operator==( const PropKey& a, const PropKey& b )
  {
    return a.hash_ == b.hash_ && ci_equal( a.name_, b.name_ );
  }

private:
  std::string_view name_;
  u32 hash_;
};

/**
 * An element's properties, in the order the file listed them.
 *
 * A flat scan rather than a tree: an element carries about a dozen properties and
 * readProperties() probes some ninety fixed names against them, so nearly every lookup misses
 * and a contiguous run of hashes beats descending a tree.
 *
 * clear() only forgets the live count, so the vectors and every string buffer in them are reused
 * by the next element. Removal leaves the slot with a 0 hash rather than shifting anything down,
 * which is what keeps its strings around to be reused.
 */
class ConfigProps
{
public:
  static constexpr size_t npos = static_cast<size_t>( -1 );

  void clear()
  {
    count_ = 0;
    live_ = 0;
    mask_ = 0;
    by_name_valid_ = false;
  }
  bool empty() const { return live_ == 0; }

  void emplace( std::string_view name, std::string_view value )
  {
    if ( count_ == entries_.size() )
    {
      entries_.emplace_back();
      hashes_.push_back( 0 );
    }
    const u32 hash = ci_hash( name );
    entries_[count_].first.assign( name );
    entries_[count_].second.assign( value );
    hashes_[count_] = hash;
    mask_ |= bit_of( hash );
    ++count_;
    ++live_;
    by_name_valid_ = false;
  }

  size_t find( PropKey key ) const
  {
    const u32 hash = key.hash();
    // readProperties() probes far more names than an element carries, so reject most of the
    // misses before the scan: a name the element never held cannot have set its bit.
    if ( !( mask_ & bit_of( hash ) ) )
      return npos;
    // The hash compare carries the search and the name compare only confirms a hit, so a
    // collision costs one strnicmp rather than a wrong answer.
    for ( size_t i = 0; i < count_; ++i )
    {
      if ( hashes_[i] == hash && ci_equal( entries_[i].first, key.name() ) )
        return i;
    }
    return npos;
  }

  /**
   * The next live slot in name order.
   *
   * That order is load-bearing: menus are built from their properties in it, and read_movecost()
   * interpolates over them.
   *
   * Sorted once and walked with a cursor rather than rescanned for the smallest each time --
   * scripts drain large elements through ReadConfigFile(), and a scan per property would be
   * quadratic. Ties keep the earlier slot, so repeated names come back in file order.
   */
  size_t take_first()
  {
    if ( !by_name_valid_ )
    {
      by_name_.clear();
      by_name_.reserve( live_ );
      for ( size_t i = 0; i < count_; ++i )
      {
        if ( hashes_[i] != 0 )
          by_name_.push_back( i );
      }
      std::stable_sort( by_name_.begin(), by_name_.end(), [this]( size_t a, size_t b )
                        { return ci_cmp_pred{}( entries_[a].first, entries_[b].first ); } );
      by_name_pos_ = 0;
      by_name_valid_ = true;
    }

    while ( by_name_pos_ < by_name_.size() )
    {
      const size_t i = by_name_[by_name_pos_++];
      if ( hashes_[i] != 0 )  // it may have been removed by name since we sorted
        return i;
    }
    return npos;
  }

  const std::string& name_at( size_t i ) const { return entries_[i].first; }
  const std::string& value_at( size_t i ) const { return entries_[i].second; }

  void remove_at( size_t i )
  {
    hashes_[i] = 0;
    --live_;
  }

  size_t estimateSize() const;

private:
  static constexpr u64 bit_of( u32 hash ) { return 1ull << ( hash & 63 ); }

  std::vector<u32> hashes_;  // 0 marks a removed slot; parallel to entries_
  std::vector<std::pair<std::string, std::string>> entries_;
  size_t count_ = 0;  // slots in use, tombstones included
  size_t live_ = 0;
  // One bit per name held, by the low bits of its hash. A removal leaves its bit set, which only
  // costs a scan that finds nothing.
  u64 mask_ = 0;

  std::vector<size_t> by_name_;  // slots in name order, only if take_first() was ever asked
  size_t by_name_pos_ = 0;
  bool by_name_valid_ = false;
};

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
  ConfigElem() = default;
  ~ConfigElem() override = default;
  size_t estimateSize() const override;
  friend class ConfigFile;

  bool has_prop( PropKey propname ) const;

  std::string remove_string( PropKey propname );
  std::string remove_string( PropKey propname, const char* dflt );

  unsigned short remove_ushort( PropKey propname );
  unsigned short remove_ushort( PropKey propname, unsigned short dflt );

  int remove_int( PropKey propname );
  int remove_int( PropKey propname, int dflt );

  unsigned remove_unsigned( PropKey propname );
  unsigned remove_unsigned( PropKey propname, int dflt );

  unsigned int remove_ulong( PropKey propname );
  unsigned int remove_ulong( PropKey propname, unsigned int dflt );

  bool remove_bool( PropKey propname );
  bool remove_bool( PropKey propname, bool dflt );

  float remove_float( PropKey propname, float dflt );
  double remove_double( PropKey propname, double dflt );

  void clear_prop( PropKey propname );

  bool remove_first_prop( std::string* propname, std::string* value );
  bool remove_prop( PropKey propname, std::string* value );
  bool remove_prop( PropKey propname, unsigned int* plong );
  bool remove_prop( PropKey propname, unsigned short* pushort );

  bool read_prop( PropKey propname, std::string* value ) const;

  // get_prop calls: don't remove, and throw if not found.
  void get_prop( PropKey propname, unsigned int* plong ) const;

  std::string read_string( PropKey propname ) const;
  std::string read_string( PropKey propname, const char* dflt ) const;


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
  [[noreturn]] void prop_not_found( std::string_view propname ) const;
  ConfigProps properties;
};

}  // namespace Pol::Clib

/// The key already carries its hash, so this hands out the one PropKey folded at compile time.
template <>
struct std::hash<Pol::Clib::PropKey>
{
  constexpr size_t operator()( const Pol::Clib::PropKey& key ) const noexcept { return key.hash(); }
};

#endif
