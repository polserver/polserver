#include <string>

#include "bscript/bobject.h"
#include "bscript/escrutil.h"
#include "plib/pkg.h"
#include "pol/testing/testenv.h"

namespace Pol::Testing
{
namespace
{
// What a script gets for a config file value: its type and how it prints.
std::string cfg_value( const std::string& text, int radix = 0 )
{
  Bscript::BObject obj( Bscript::bobject_from_string( text, radix ) );
  return std::string( obj->typeOf() ) + ": " + obj->getStringRep();
}
}  // namespace

void cfg_value_test()
{
  // A config file value is read as a number when all of it is one, and as a string otherwise.
  // Every ReadConfigFile() property goes through this, so each shape here is a type a script sees.
  const auto check = []( const std::string& text, const std::string& expected, int radix = 0 )
  {
    UnitTest( [&] { return cfg_value( text, radix ); }, expected,
              "config value \"" + text + "\" reads as " + expected );
  };
  check( "42", "Integer: 42" );
  check( "-42", "Integer: -42" );
  check( "+7", "Integer: 7" );
  check( "0x1F", "Integer: 31" );
  check( "42 // a comment", "Integer: 42" );
  check( "2147483647", "Integer: 2147483647" );
  check( "-2147483648", "Integer: -2147483648" );
  check( "2147483648", "String: 2147483648" );
  check( "-2147483649", "String: -2147483649" );
  check( "99999999999999999999", "String: 99999999999999999999" );
  check( "1.5", "Double: 1.5" );
  check( "-0.25", "Double: -0.25" );
  check( "42abc", "String: 42abc" );
  check( "1.2.3", "String: 1.2.3" );
  check( "abc", "String: abc" );
  check( "-", "String: -" );
  // Two long-standing readings a shard may rely on: a leading zero is octal, and scientific
  // notation is not a number.
  check( "010", "Integer: 8" );
  check( "1e3", "String: 1e3" );
  // Keys and some properties are read as hex whether or not they carry 0x.
  check( "1F", "Integer: 31", 16 );

  // Package versions: dot separated numbers compared part by part.
  const auto newer_or_equal = []( const std::string& have, const std::string& need, bool expected )
  {
    UnitTest( [&] { return Plib::version_greater_or_equal( have, need ); }, expected,
              "version " + have + ( expected ? " satisfies " : " does not satisfy " ) + need );
  };
  newer_or_equal( "0", "0", true );
  newer_or_equal( "1", "0", true );
  newer_or_equal( "0", "1", false );
  newer_or_equal( "0.5", "1", false );
  newer_or_equal( "0.5", "0", true );
  newer_or_equal( "1.2", "1.12", false );
  newer_or_equal( "1.12", "1.2", true );
  newer_or_equal( "1.2.3", "1", true );
  newer_or_equal( "1.1", "1.2.3", false );
  newer_or_equal( "1.3", "1.2.3", true );
  // a part with a leading zero is a decimal number, not an octal one
  newer_or_equal( "1.08", "1.7", true );
  newer_or_equal( "1.7", "1.08", false );
  newer_or_equal( "2.010", "2.9", true );

  UnitTest( [] { return Plib::version_equal( "93", "93.0.0" ); }, true,
            "a missing version part counts as 0" );
  UnitTest( [] { return Plib::version_equal( "93.0.0", "93" ); }, true,
            "and so do trailing zero parts" );
  UnitTest( [] { return Plib::version_equal( "1.08", "1.8" ); }, true,
            "1.08 and 1.8 are the same version" );
}
}  // namespace Pol::Testing
