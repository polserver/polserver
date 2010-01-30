/*
History
=======
2008/07/08 Turley: Added Random Number Generator "lagged Fibonacci generator"

Notes
=======

*/

/*
SGPC: Simple Genetic Programming in C
(c) 1993 by Walter Alden Tackett and Aviram Carmi
 
 This code and documentation is copyrighted and is not in the public domain.
 All rights reserved. 
 
 - This notice may not be removed or altered.
 
 - You may not try to make money by distributing the package or by using the
   process that the code creates.
 
 - You may not distribute modified versions without clearly documenting your
   changes and notifying the principal author.
 
 - The origin of this software must not be misrepresented, either by
   explicit claim or by omission.  Since few users ever read sources,
   credits must appear in the documentation.
 
 - Altered versions must be plainly marked as such, and must not be
   misrepresented as being the original software.  Since few users ever read
   sources, credits must appear in the documentation.
 
 - The authors are not responsible for the consequences of use of this 
   software, no matter how awful, even if they arise from flaws in it.
 
If you make changes to the code, or have suggestions for changes,
let us know!  (gpc@ipld01.hac.com)
*/

#ifndef _RANDOM_H_
#define _RANDOM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rawtypes.h"
	
extern int 	gaussian_noise_toggle;
extern float 	gaussian_noise_uniform1, gaussian_noise_uniform2;
extern float	gaussian_noise_temp;

/* random.c */

float random_float ( float f );

int random_int ( int i );

int random_int_range (int minI,int maxI);

void set_seed ( unsigned long s );

unsigned long get_seed (void);

float park_miller_randomizer (void);

long PMrand ( long *s );

float gaussian_noise (   float mean,    float sigma );


// Random Number Generator
 
// Ranq1 generates a 64-bit random
// period of 1.8+10^19
// only used for initialize Ranfib's table of 55 random values.
struct Ranq1
{
	u64 v;
	Ranq1(u64 j): v(4101842887655102017ULL)
	{
		v ^=j;
		v=int64();
	}
	inline u64 int64()
	{
		v ^= v >>21;
		v ^= v << 35;
		v ^= v >> 4;
		return v * 2685821657736338717ULL;
	}
	inline double doub() { return 5.42101086242752217E-20 * int64(); }
};


// The 'real' random number Generator
// variant of Knuth's subtractive generator
// "lagged Fibonacci generator"
// generates new values directly als floating point, 
// by the floating-point subtraction of two previos values.
struct Ranfib
{
	double dtab[55],dd;
	int inext, inextp;
	Ranfib(u64 j):inext(0), inextp(31){  // Constructor Call with any integer seed
		Ranq1 init(j);
		for (int k=0;k<55;k++) dtab[k] = init.doub();
	}
	double doub()  // Returns random double-precision floating value between 0. and 1.
	{
		if (++inext >=55) inext=0;
		if (++inextp>=55) inextp=0;
		dd = dtab[inext] - dtab[inextp];
		if (dd<0) dd += 1.0;
		return (dtab[inext] =dd);
	}
};



#ifdef __cplusplus
}
#endif

#endif
