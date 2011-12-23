/*
History
=======
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <stddef.h>

#include "../clib/endian.h"

#include "msghandl.h"
#include "network/packets.h"
#include "pktin.h"
#include "pktout.h"
#include "tiplist.h"
#include "sockio.h"

#include <cstring>

TipFilenames tipfilenames;

bool send_tip( Client* client, const char* tipname, unsigned short tipnum )
{
    size_t textlen = strlen(tipname);
    if ( textlen > 0 && unsigned(textlen) <= 9999)
    {
		PktHelper::PacketOut<PktOut_A6> msg;
		msg->WriteFlipped(static_cast<u16>(textlen+11));
		msg->Write(static_cast<u8>(PKTOUT_A6_TYPE_TIP));
		msg->offset+=2; //unk4,5
		msg->WriteFlipped(tipnum);
		msg->WriteFlipped(static_cast<u16>(textlen+1));
		msg->Write(tipname,static_cast<u16>(textlen+1));
		msg.Send(client);
        return true;
    }
    else
    {
        return false;
    }
}

void send_tip( Client* client, const std::string& tiptext )
{
    size_t textlen = tiptext.size();
    if (textlen >= 10000)
        textlen = 9999;

	PktHelper::PacketOut<PktOut_A6> msg;
	msg->WriteFlipped(static_cast<u16>(textlen+11));
	msg->Write(static_cast<u8>(PKTOUT_A6_TYPE_TIP));
	msg->offset+=2; //unk4,5
	msg->offset+=2; //tipnum
	msg->WriteFlipped(static_cast<u16>(textlen+1));
	msg->Write(tiptext.c_str(),static_cast<u16>(textlen+1));
	msg.Send(client);
}

void handle_get_tip( Client* client, PKTIN_A7* msg )
{
	u16 tipnum = cfBEu16( msg->lasttip );
    if (!tipfilenames.empty())
    {
        if (msg->prevnext)
        {
            ++tipnum;
            if (tipnum >= tipfilenames.size())
                tipnum = 0;
        }
        else
        {
            --tipnum;
            if (tipnum >= tipfilenames.size())
                tipnum = static_cast<u16>(tipfilenames.size()) - 1;
        }

        send_tip( client, tipfilenames[tipnum].c_str(), tipnum );
    }
}

MESSAGE_HANDLER( PKTIN_A7, handle_get_tip );
