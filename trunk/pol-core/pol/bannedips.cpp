/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"
#include "client.h"
#include "bannedips.h"

bool is_banned_ip(Client* client)
{
    if (banned_ips.empty())
        return false;

	for( std::vector<IPRule>::iterator itr = banned_ips.begin(); itr != banned_ips.end(); ++itr )
    {
		unsigned long addr1part, addr2part;
        struct sockaddr_in* sockin = reinterpret_cast<struct sockaddr_in*>(&client->ipaddr);
		addr1part = itr->ipMatch & itr->ipMask;

#ifdef _WIN32
        addr2part = sockin->sin_addr.S_un.S_addr & itr->ipMask;
#else
        addr2part = sockin->sin_addr.s_addr      & itr->ipMask;
#endif

        if (addr1part == addr2part)
            return true;
    }
	return false;
}

void read_bannedips_config( bool initial_load)
{
	if (!initial_load)
	{
		banned_ips.clear();
	}
    if (!FileExists( "config/bannedips.cfg" ))
        return;

    ConfigFile cf( "config/bannedips.cfg" );
    ConfigElem elem;

	while (cf.read( elem ))
    {
		IPRule CurrentEntry;
		string iptext = elem.remove_string( "IPMatch" );
		string::size_type delim = iptext.find_first_of( "/" );
		if (delim != string::npos)
		{
			string ipaddr_str = iptext.substr( 0, delim );
			string ipmask_str = iptext.substr( delim+1 );
			CurrentEntry.ipMatch = inet_addr( ipaddr_str.c_str() );
			CurrentEntry.ipMask = inet_addr( ipmask_str.c_str() );
			banned_ips.push_back(CurrentEntry);
		}
		else
		{
			string ipmask_str = "255.255.255.255";
			CurrentEntry.ipMatch = inet_addr( iptext.c_str() );
			CurrentEntry.ipMask = inet_addr( ipmask_str.c_str() );
			banned_ips.push_back(CurrentEntry);
		}
	}
}
