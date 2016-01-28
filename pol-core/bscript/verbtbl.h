/** @file
 *
 * @par History
 */


#ifndef __VERBTBL_H
#define __VERBTBL_H
namespace Pol {
  namespace Bscript {
	class Verb
	{
	public:
	  char *verb;
	  int narg;

	  Verb( const char *name, int nargs );
	};

	int isInTable( Verb *table, int tableSize, const char *search, Verb **result );
  }
}
#endif
