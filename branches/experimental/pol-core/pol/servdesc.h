/*
History
=======


Notes
=======

*/

#ifndef SERVDESC_H
#define SERVDESC_H

#include <string>
#include <vector>

class ServerDescription
{
public:
    ServerDescription();

    std::string name;
	unsigned char ip[4];
	unsigned short port;
    std::vector<unsigned int> ip_match;
    std::vector<unsigned int> ip_match_mask;
	std::vector<string> acct_match;
    std::string hostname;
};

#endif
