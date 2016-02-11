#ifndef CLIB_UNITTEST_H
#define CLIB_UNITTEST_H
namespace Pol {
  namespace Clib {
	class UnitTest
	{
	public:
	  UnitTest( void( *testfunc )( ) ) { ( *testfunc )( ); }
	};
  }
}
#endif
