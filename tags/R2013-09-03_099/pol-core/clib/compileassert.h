/*
History
=======

2009/08/25 Shinigami: STLport-5.2.1 fix: additional parentheses in asserteql
2010/03/27 Shinigami: Notes added.

Notes
=======

How does it work?
  asserteql
    defines one "char xxxyyyzzz[1];" globaly.
    per check it tries to define another "char xxxyyyzzz[1-(0)];" (a^(b) = a XOR b).
      if this is not possible, than it will fail, due to different definitions.

  assertsize
    tries to define two entries in a different way.
	  if this is not possible, than it will fail, due to different definitions.

  assertofs
    will not be used.

  Why we have asserteql and assertsize if it does the same? dunno

*/

#ifndef CLIB_COMPILEASSERT_H
#define CLIB_COMPILEASSERT_H

extern char xxxyyyzzz[1];
#if _lint
#	define asserteql(a,b) extern char xxxyyyzzz[1]
#	define assertofs(type,var,pos) extern char xxxyyyzzz[1]
#else
#	define asserteql(a,b) extern char xxxyyyzzz[ (unsigned)1 - (unsigned)(a^(b)) ]
#	define assertofs(type,var,pos) asserteql( offsetof(type,var),pos )
#endif
#define arsize(a) (sizeof a / sizeof a[0])

#define assertsize(type,length)  \
        extern char size_##type[ length ]; \
        extern char size_##type[ sizeof(type) ]

#endif
