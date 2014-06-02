/*
History
=======
2005/06/01 Shinigami: added getstatics - to fill a list with statics

Notes
=======

*/

#ifndef PLIB_STATICSERVER_H
#define PLIB_STATICSERVER_H

#include "realmdescriptor.h"
namespace Pol {
  namespace Plib {
	struct STATIC_INDEX;
	struct STATIC_ENTRY;

	class StaticServer
	{
	public:
	  explicit StaticServer( const RealmDescriptor& descriptor );
	  ~StaticServer();
	  StaticServer & operator=( const StaticServer & ) { return *this; }

	  bool findstatic( unsigned short x, unsigned short y, unsigned short objtype ) const;
	  void getstatics( StaticEntryList& statics, unsigned short x, unsigned short y ) const;
      size_t memorySize() const;
	protected:
	  void Validate() const;
	  void ValidateBlock( unsigned short x, unsigned short y ) const;

	private:
	  RealmDescriptor _descriptor;

	  vector<STATIC_INDEX> _index;
	  vector<STATIC_ENTRY> _statics;
	};
  }
}
#endif
