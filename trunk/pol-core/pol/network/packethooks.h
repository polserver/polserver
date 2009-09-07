/*
History
=======
2009/08/03 MuadDib:   Upgraded ph6017 and is6017 to version, and integer instead of bool.

Notes
=======
Version member: Positive Integer. This is used to translate the "version" of the packet structure
to the correct internal core Message Handler (Default 1, which translates to use handler[]). Each
new Handler added to the core needs a new Version number here. As of 8/3/09 there is only 2.

*/

#ifndef PACKETHOOKS_H
#define PACKETHOOKS_H

#include "../../clib/stl_inc.h"

#include "client.h"
#include "../packetscrobj.h"
#include "../syshook.h"

class PacketHookData 
{
public:
    PacketHookData() :
        length(0),
        function(NULL),
        outgoing_function(NULL),
        default_handler(NULL),
        sub_command_offset(0),
        sub_command_length(0),
		version(0)
    {
		memset( &client_ver, 0, sizeof(client_ver) );
	};
	~PacketHookData();

    int length; // if 0, variable length
    ExportedFunction* function;
    ExportedFunction* outgoing_function;
    void(*default_handler)(Client *client, void *msg);
    unsigned short sub_command_offset;
    unsigned short sub_command_length;
	unsigned short version;
	VersionDetailStruct client_ver;
    map<u32,PacketHookData*>SubCommands;
};

extern std::vector<PacketHookData> packet_hook_data;
extern std::vector<PacketHookData> packet_hook_data_v2;
void load_packet_hooks();
void ExportedPacketHookHandler(Client* client, void* data);
void CallOutgoingPacketExportedFunction(Client* client, const void*& data, int& inlength, ref_ptr<BPacket>& outpacket, bool& handled);
void clean_packethooks();

void SetVersionDetailStruct( const std::string& ver, VersionDetailStruct& detail );
bool CompareVersionDetail(VersionDetailStruct ver1, VersionDetailStruct ver2);

#endif
