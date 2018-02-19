#pragma once

#include <string>

// The hash function below is necessary for UOP decoding and
// was modified from C# to C++ from the code provided in LegacyMULConverter 
// by Eos, which claims comes from http://burtleburtle.net/bob/c/lookup3.c. (Nando)
//
uint64_t HashLittle2( std::string s )
{
  int length = s.size();

  uint32_t a, b, c;
  a = b = c = 0xDEADBEEF + (uint32_t)length;

  int k = 0;

  while ( length > 12 )
  {
    a += s[k];
    a += ((uint32_t)s[k + 1]) << 8;
    a += ((uint32_t)s[k + 2]) << 16;
    a += ((uint32_t)s[k + 3]) << 24;
    b += s[k + 4];
    b += ((uint32_t)s[k + 5]) << 8;
    b += ((uint32_t)s[k + 6]) << 16;
    b += ((uint32_t)s[k + 7]) << 24;
    c += s[k + 8];
    c += ((uint32_t)s[k + 9]) << 8;
    c += ((uint32_t)s[k + 10]) << 16;
    c += ((uint32_t)s[k + 11]) << 24;

    a -= c; a ^= ((c << 4) | (c >> 28)); c += b;
    b -= a; b ^= ((a << 6) | (a >> 26)); a += c;
    c -= b; c ^= ((b << 8) | (b >> 24)); b += a;
    a -= c; a ^= ((c << 16) | (c >> 16)); c += b;
    b -= a; b ^= ((a << 19) | (a >> 13)); a += c;
    c -= b; c ^= ((b << 4) | (b >> 28)); b += a;

    length -= 12;
    k += 12;
  }

  if ( length != 0 )
  {
    switch ( length )
    {
    case 12: c += ((uint32_t)s[k + 11]) << 24; /* fall through */
    case 11: c += ((uint32_t)s[k + 10]) << 16; /* fall through */
    case 10: c += ((uint32_t)s[k + 9]) << 8; /* fall through */
    case 9: c += s[k + 8]; /* fall through */
    case 8: b += ((uint32_t)s[k + 7]) << 24; /* fall through */
    case 7: b += ((uint32_t)s[k + 6]) << 16; /* fall through */
    case 6: b += ((uint32_t)s[k + 5]) << 8; /* fall through */
    case 5: b += s[k + 4]; /* fall through */
    case 4: a += ((uint32_t)s[k + 3]) << 24; /* fall through */
    case 3: a += ((uint32_t)s[k + 2]) << 16; /* fall through */
    case 2: a += ((uint32_t)s[k + 1]) << 8; /* fall through */
    case 1: a += s[k]; break;
    }

    c ^= b; c -= ((b << 14) | (b >> 18));
    a ^= c; a -= ((c << 11) | (c >> 21));
    b ^= a; b -= ((a << 25) | (a >> 7));
    c ^= b; c -= ((b << 16) | (b >> 16));
    a ^= c; a -= ((c << 4) | (c >> 28));
    b ^= a; b -= ((a << 14) | (a >> 18));
    c ^= b; c -= ((b << 24) | (b >> 8));
  }

  return ((uint64_t)b << 32) | c;
}