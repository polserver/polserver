/*
History
=======

Notes
=======

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "fdump.h"


static const char *dump16( const void *data, int len )
{
	int i;
    static char buf[80];
	char temp[10];
	const unsigned char *s = (const unsigned char *) data;
	buf[0] = '\0';

	for(i = 0; i < 16; i++) 
	{
		if (i < len)
			sprintf( temp, "%2.02x ", s[i] );
		else
			strcpy( temp, "   " );
		strcat( buf, temp );
		if (i == 7) 
			strcat( buf, " " );
    }
	strcat( buf, "  " );
	temp[1] = '\0';
	for(i = 0; i < 16; i++) 
	{
		if (i >= len || !isprint(s[i])) 
			temp[0] = '.';
		else 
			temp[0] = s[i];
		strcat( buf, temp );
		if (i == 7) 
			strcat( buf, " " );
	}
	return buf;
}

void fdump( FILE *fp, const void *data, int len )
{
	int i;
	const unsigned char *s = (const unsigned char *) data;

    for(i = 0; i < len; i += 16) {
		int nprint = len - i;
		if (nprint > 16) nprint = 16;
		fprintf(fp, "%4.04x: %s\n", i, dump16( &s[i], nprint ) );
	}
}

