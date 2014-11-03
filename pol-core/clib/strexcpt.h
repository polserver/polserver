/*
History
=======

Notes
=======

*/

#ifndef __STREXCPT_H
#define __STREXCPT_H

#include <stdexcept>

// set_terminate
// set_unexpected
// set_se_handler
namespace Pol {
  namespace Clib {
	class structured_exception : public std::runtime_error
	{
	public:
	  unsigned int nSE;
	public:
	  structured_exception( unsigned int n, const char *text = "Structured Exception" ) :
		runtime_error( text ),
		nSE( n )
	  {}
	};

	class access_violation : public structured_exception
	{
	public:                     // FIXME icky constant
	  access_violation() : structured_exception( 5, "Access Violation" ) {}
	};

	void InstallStructuredExceptionHandler( void );
	void InstallOldStructuredExceptionHandler( void );

  }
}
#endif
