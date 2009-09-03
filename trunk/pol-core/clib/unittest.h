#ifndef CLIB_UNITTEST_H
#define CLIB_UNITTEST_H

class UnitTest
{
public:
    UnitTest( void (*testfunc)() ) { (*testfunc)(); }
};

#endif
