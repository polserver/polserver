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

struct STATIC_INDEX;
struct STATIC_ENTRY;

class StaticServer
{
public:
    explicit StaticServer( const RealmDescriptor& descriptor );
    ~StaticServer();
	StaticServer & operator=( const StaticServer & ) { return *this; }

    bool findstatic( unsigned x, unsigned y, unsigned short objtype ) const;
    void getstatics( StaticEntryList& statics, unsigned x, unsigned y ) const;

protected:
    void Validate() const;
    void ValidateBlock( unsigned x, unsigned y ) const;

private:
    RealmDescriptor _descriptor;

    vector<STATIC_INDEX> _index;
    vector<STATIC_ENTRY> _statics;
};

#endif
