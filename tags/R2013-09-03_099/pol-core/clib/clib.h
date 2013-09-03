/*
History
=======

Notes
=======

*/

#ifndef __CLIB_H
#define __CLIB_H


#ifndef __STDDEF_H
#	include "stddef.h"
#endif

    /* explicit cast: use instead of c-style (type) casts.
       Specify what you want to cast to, and 
       what you think you're casting from.
    */
#define EXPLICIT_CAST(totype,fromtype) (totype)

#define ms_to_clocks(ms) (ms * CLOCKS_PER_SEC / 1000)

/* how many file handles are available? */
int favail(void);

/* make a space-padded string */
char *strpcpy(char *dest, const char *src, size_t maxlen);

/* make an always null terminated string in maxlen characters */
char *stracpy(char *dest, const char *src, size_t maxlen);
#define strzcpy stracpy

/* do first strlen(b) chars of a and b match? match(s, "tag") */
int match(char *a, char *b);

/* strip leading, trailing spaces, trim multile whitespace to single space */
char *strip(char *s);

/* open a unique file in a directory, no sharing allowed, "w+b" */
#ifdef __STDIO_H    /* well there you go, not portable... why not standard the idempotent mechanism? */
FILE *uniqfile(char *directory, char *pathname);
#endif
/* perform a function on all files meeting a file spec */
int forspec(const char *spec, void (*func)(const char *pathname));

/* make a char** array suitable for call to exec() or spawn().
   return the argc in *pargc. 
*/
char **decompose1(char *cmdline, int *pargc);

/* remove all occurrences of  characters not in allowed from s */
void legalize(char *s, const char *allowed);

/* how many words in s? nondestructive, assumes s is strip()ed */
int nwords(const char *s);

/* return nbits binary representation of val, in static buffer */
char *binary(unsigned int val, int nbits);

/* take a string, increment alphas, 'ZZZZ' converts to 'AAAA', etc */
void incStr(char *str);

#define STRCPY(a,b)  strncpy(a,b,sizeof a)
#define STRNCPY(a,b) strncpy(a,b,sizeof a)
#define STRPCPY(a,b) strpcpy(a,b,sizeof a)
#define STRACPY(a,b) stracpy(a,b,sizeof a)
#define MEMCLR(a)     memset(&a,0,sizeof a)
#define MEMCPY(a,b)   memcpy(&a,&b,sizeof a)


/* GCCC is soo weak... */
#ifdef __GNUC__
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif


#ifdef __cplusplus
#define PARAMS ...
#else
#define PARAMS
#endif

#if 0
/* critical error handler.  always returns failure. */
/* now fopen("A:\\test.txt", "rt") returns NULL if no disk in A.
   without doing the abort retry fail crap
*/
void interrupt newcrit(PARAMS);
#endif


#endif
