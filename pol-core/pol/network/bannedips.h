/*
History
=======
2009/01/24 MuadDib   Creation of bannedips.h. This is for the new upcoming bannedips.cpp/h setup.
                     This will work by storing the ip/mask in a vectored struct so
                     it's not reloading EVERY client connection! Will need to set a
                     clearing/reset with reloadcfg for this new system when done.

Notes
=======

*/

#include <string>
#include <vector>

struct IPRule
{
	unsigned int ipMatch;
	unsigned int ipMask;
};

std::vector< IPRule > banned_ips;

