/*
History
=======
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID

Notes
=======

*/

#include "clib/stl_inc.h"

#include <stddef.h>

#include "clib/endian.h"

#include "msghandl.h"
#include "pktin.h"
#include "pktout.h"
#include "tiplist.h"
#include "sockio.h"

TipFilenames tipfilenames;

static PKTOUT_A6 tipmsg;

bool send_tip( Client* client, const char* tipname, unsigned short tipnum )
{
    ifstream ifs( tipname );
    ifs.read( tipmsg.text, sizeof tipmsg.text );
    streamsize textlen = ifs.gcount();
    if ( textlen > 0 && unsigned(textlen) <= sizeof tipmsg.text)
    {
        u16 msglen = static_cast<u16>(offsetof(PKTOUT_A6, text) + textlen);

        tipmsg.msgtype = PKTOUT_A6_ID;
        tipmsg.msglen = ctBEu16(msglen);
        tipmsg.type = PKTOUT_A6::TYPE_TIP;
        tipmsg.unk4 = 0;
        tipmsg.unk5 = 0;
        tipmsg.tipnum = ctBEu16(tipnum);
        tipmsg.textlen = ctBEu16(textlen);
        // tipmsg.text read in above

        transmit( client, &tipmsg, msglen );
        return true;
    }
    else
    {
        return false;
    }
}

void send_tip( Client* client, const std::string& tiptext )
{
    unsigned textlen = tiptext.size();
    if (textlen >= sizeof tipmsg.text)
        textlen = sizeof tipmsg.text - 1;
    
    int msglen = offsetof(PKTOUT_A6, text) + textlen;

    tipmsg.msgtype = PKTOUT_A6_ID;
    tipmsg.msglen = ctBEu16(msglen);
    tipmsg.type = PKTOUT_A6::TYPE_TIP;
    tipmsg.unk4 = 0;
    tipmsg.unk5 = 0;
    tipmsg.tipnum = 0;
    tipmsg.textlen = ctBEu16(textlen);
    memcpy( tipmsg.text, tiptext.c_str(), textlen );
    tipmsg.text[textlen] = '\0';

    transmit( client, &tipmsg, msglen );
}

void handle_get_tip( Client* client, PKTIN_A7* msg )
{
	u16 tipnum = cfBEu16( msg->lasttip );
    if (tipfilenames.size())
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
