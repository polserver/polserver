#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "pol/network/client.h"
#include "pol/network/packethooks.h"
#include "pol/proplist.h"
#include "pol/testing/testenv.h"

namespace Pol::Testing
{
namespace
{
using Core::CPropProfiler;
using Core::PropertyList;

PropertyList make( std::initializer_list<std::pair<const char*, const char*>> props )
{
  PropertyList list( CPropProfiler::Type::ITEM );
  for ( const auto& [name, value] : props )
    list.setprop( name, value );
  return list;
}

// The property's value, or "<missing>".
std::string value_of( const PropertyList& list, const std::string& name )
{
  std::string value;
  if ( !list.getprop( name, value ) )
    return "<missing>";
  return value;
}

std::string names_of( const PropertyList& list )
{
  std::vector<std::string> names;
  list.getpropnames( names );
  std::string res;
  for ( const auto& name : names )
    res += name + " ";
  return res;
}

// getpropnames lists in stored order, and lookups binary search that order: it has to be sorted
// and free of duplicates for every property to be found.
bool names_ordered( const PropertyList& list )
{
  std::vector<std::string> names;
  list.getpropnames( names );
  for ( const auto& name : names )
  {
    std::string value;
    if ( !list.getprop( name, value ) )
      return false;
  }
  std::set<std::string> unique( names.begin(), names.end() );
  return unique.size() == names.size();
}
}  // namespace

void proplist_test()
{
  // copyprops merges two sorted lists: names from both sides interleave, and where both have a
  // name the copied list's value wins. This is how an item takes its itemdesc cprops and a
  // corpse its character's.
  UnitTest(
      []
      {
        auto dest = make( { { "b", "s1" }, { "d", "s2" }, { "f", "s3" } } );
        auto from = make( { { "a", "s9" }, { "b", "s5" }, { "e", "s7" }, { "g", "s8" } } );
        dest.copyprops( from );
        return value_of( dest, "a" ) + value_of( dest, "b" ) + value_of( dest, "d" ) +
               value_of( dest, "e" ) + value_of( dest, "f" ) + value_of( dest, "g" );
      },
      std::string( "s9s5s2s7s3s8" ), "copyprops merges and the copied value wins" );
  UnitTest(
      []
      {
        auto dest = make( { { "b", "s1" }, { "d", "s2" }, { "f", "s3" } } );
        auto from = make( { { "a", "s9" }, { "b", "s5" }, { "e", "s7" }, { "g", "s8" } } );
        dest.copyprops( from );
        return names_ordered( dest ) && names_of( dest ) == "a b d e f g ";
      },
      true, "copyprops leaves one of each name, in order" );
  UnitTest(
      []
      {
        auto dest = make( {} );
        auto from = make( { { "x", "s1" }, { "y", "s2" } } );
        dest.copyprops( from );
        return dest == from;
      },
      true, "copyprops into an empty list copies it whole" );
  UnitTest(
      []
      {
        auto dest = make( { { "x", "s1" } } );
        dest.copyprops( make( {} ) );
        return names_of( dest ) + value_of( dest, "x" );
      },
      std::string( "x s1" ), "copyprops of an empty list changes nothing" );

  // operator- erases the named properties and ignores names that are not there.
  UnitTest(
      []
      {
        auto list = make( { { "a", "s1" }, { "b", "s2" }, { "c", "s3" } } );
        list - std::set<std::string>{ "a", "c", "missing" };
        return names_of( list ) + value_of( list, "b" );
      },
      std::string( "b s2" ), "operator- erases the named properties" );

  // A packet hook with a Client version applies to that version and later ones.
  using Network::VersionDetailStruct;
  const VersionDetailStruct v7090{ 7, 0, 9, 0 };
  UnitTest( [&] { return Network::CompareVersionDetail( v7090, v7090 ); }, true,
            "CompareVersionDetail counts the same version as newer or equal" );
  UnitTest( [&] { return Network::CompareVersionDetail( { 7, 0, 10, 0 }, v7090 ); }, true,
            "CompareVersionDetail compares the revision as a number, not text" );
  UnitTest( [&] { return Network::CompareVersionDetail( { 7, 0, 9, 1 }, v7090 ); }, true,
            "CompareVersionDetail counts a later patch as newer" );
  UnitTest( [&] { return Network::CompareVersionDetail( { 6, 9, 99, 99 }, v7090 ); }, false,
            "CompareVersionDetail lets the major version decide first" );
  UnitTest( [&] { return Network::CompareVersionDetail( { 7, 0, 8, 99 }, v7090 ); }, false,
            "CompareVersionDetail counts an earlier revision as older" );

  // The Client version of packethooks.cfg: four numbers, or before 5.0.7 three and a letter.
  const auto parsed = []( const std::string& ver )
  {
    VersionDetailStruct detail{};
    Network::SetVersionDetailStruct( ver, detail );
    return fmt::format( "{}.{}.{}.{}", detail.major, detail.minor, detail.rev, detail.patch );
  };
  UnitTest( [&] { return parsed( "7.0.16.3" ); }, std::string( "7.0.16.3" ),
            "SetVersionDetailStruct reads four numbers" );
  UnitTest( [&] { return parsed( "7.0.9" ); }, std::string( "7.0.9.0" ),
            "SetVersionDetailStruct reads a missing patch as 0" );
  UnitTest( [&] { return parsed( "4.0.7a" ); }, std::string( "4.0.7.1" ),
            "SetVersionDetailStruct reads an old letter patch, a as 1" );
  UnitTest( [&] { return parsed( "5.0.6e" ); }, std::string( "5.0.6.5" ),
            "SetVersionDetailStruct reads an old letter patch, e as 5" );
  UnitTest( [&] { return parsed( "4.0.11c" ); }, std::string( "4.0.11.3" ),
            "SetVersionDetailStruct reads the letter patch of any version before 5.0.7" );
  UnitTest( [&] { return parsed( "5.0.7.2" ); }, std::string( "5.0.7.2" ),
            "SetVersionDetailStruct reads 5.0.7 on as four numbers" );
}
}  // namespace Pol::Testing
