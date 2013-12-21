/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../pol/uconst.h"
#include "../pol/uobject.h"
#include "../pol/los.h"
#include "../pol/npc.h"
#include "../pol/item/item.h"

#include "realm.h"
#include "mapserver.h"
#include "testenv.h"
namespace Pol {
  namespace Plib {
	void inc_successes();
	void inc_failures();

	extern Realm* p_test_realm;

	static void test_los( Core::UObject* src, Core::UObject* target, bool should_have_los )
	{
	  if ( p_test_realm == NULL )
		p_test_realm = new Realm( "britannia" );

	  cout << "LOS test: " << src->name() << " to "
		<< target->name() << "(" << should_have_los << "):";
	  bool res = p_test_realm->has_los( *src, *target );
	  cout << res << " ";
	  if ( should_have_los == res )
	  {
		cout << "Ok!" << endl;
		inc_successes();
	  }
	  else
	  {
		cout << "Failure!" << endl;
		inc_failures();
	  }

	}

	static void test_los( u16 x1, u16 y1, s8 z1,
						  u16 x2, u16 y2, s8 z2,
						  bool should_have_los )
	{
	  if ( p_test_realm == NULL )
		p_test_realm = new Realm( "britannia" );

	  cout << "LOS test: (" << x1 << "," << y1 << "," << int( z1 ) << ")"
		<< " - (" << x2 << "," << y2 << "," << int( z2 ) << ")"
		<< " (" << should_have_los << "):";
      bool res = p_test_realm->has_los( Core::LosObj( x1, y1, z1 ), Core::LosObj( x2, y2, z2 ) );
	  cout << res << " ";
	  if ( should_have_los == res )
	  {
		cout << "Ok!" << endl;
		inc_successes();
	  }
	  else
	  {
		cout << "Failure!" << endl;
		inc_failures();
	  }

	}
	static void test_los( u16 x1, u16 y1, s8 z1, u8 h1,
						  u16 x2, u16 y2, s8 z2, u8 h2,
						  bool should_have_los )
	{
	  if ( p_test_realm == NULL )
		p_test_realm = new Realm( "britannia" );

	  cout << "LOS test: (" << x1 << "," << y1 << "," << int( z1 ) << ",ht=" << int( h1 ) << ")"
		<< " - (" << x2 << "," << y2 << "," << int( z2 ) << ",ht=" << int( h2 ) << ")"
		<< " (" << should_have_los << "):";
      bool res = p_test_realm->has_los( Core::LosObj( x1, y1, z1, h1 ), Core::LosObj( x2, y2, z2, h2 ) );
	  cout << res << " ";
	  if ( should_have_los == res )
	  {
		cout << "Ok!" << endl;
		inc_successes();
	  }
	  else
	  {
		cout << "Failure!" << endl;
		inc_failures();
	  }

	}
	void pol_los_test()
	{
	  cout << "POL datafile LOS tests:" << endl;

	  test_los( 5421, 78, 10, 5422, 89, 20, false );
	  test_los( 1403, 1624, 28, 1402, 1625, 28, true );

	  test_los( 1374, 1625, 59, 1379, 1625, 30, true );
	  test_los( 1373, 1625, 59, 1379, 1625, 30, true );
	  test_los( 1372, 1625, 59, 1379, 1625, 30, false );
	  test_los( 1372, 1625, 59, 1381, 1625, 30, true );

	  // from you standing in the house in GA to a backpack outside
	  test_los( 5987, 1999, 7 + 9, 5991, 2001, 1, false );
	  test_los( 5987, 1999, 7 + 9, 5993, 1997, 1, false );

	  // test looking through "black space" in a cave
	  test_los( 5583, 226, 0, 5590, 223, 0, false );

	  test_los( test_banker2, test_chest1, true );
	  test_los( test_banker2, test_chest2, true );

	  // standing aboveground in buc's den, and below

	  test_los( test_orclord, test_banker3, false );

	  test_los( 579, 2106, 0, 1, 563, 2107, 0, 1, false );
	  test_los( 862, 1691, 0, 1, 843, 1689, 0, 1, true );
	  test_los( 1618, 3351, 3, 1, 1618, 3340, 4, 1, true );
	}
  }
}