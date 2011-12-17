/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../bscript/berror.h"
#include "../bscript/impstr.h"

#include "../clib/endian.h"
#include "../clib/rawtypes.h"

#include "item/item.h"
#include "mobile/charactr.h"
#include "module/osmod.h"
#include "module/uomod.h"
#include "msghandl.h"
#include "network/cgdata.h"
#include "network/client.h"
#include "network/packets.h"
#include "pktboth.h"
#include "sockio.h"
#include "ufunc.h"
#include "uoexec.h"

void send_prompt( Client* client, u32 serial )
{
	PktOut_9A* msg = PktHelper::RequestPacket<PktOut_9A>(PKTBI_9A_ID);
	msg->WriteFlipped(static_cast<u16>(sizeof msg->buffer));
	msg->Write(serial);
	msg->WriteFlipped(static_cast<u32>(0x15));
	msg->offset+=5; // u32 type u8 text[0]
    transmit( client, &msg->buffer, msg->offset );
	PktHelper::ReAddPacket(msg);
}

void handle_prompt( Client* client, PKTBI_9A* msg )
{
    UOExecutorModule* uoemod = client->gd->prompt_uoemod;
    if (uoemod == NULL)
        return;
    int textlen = cfBEu16( msg->msglen ) - offsetof(PKTBI_9A,text);
    if (msg->type)
    {
        if (textlen <= 120 && msg->text[textlen-1] == '\0')
        {
            bool ok = true;
            --textlen; // don't include null terminator (already checked)
            for( int i = 0; i < textlen; ++i )
            {
                if (!isprint(msg->text[i]))
                {
                    ok = false;
                    break;
                }
            }
            if (ok)
            {
                String* str = new String( msg->text, textlen );
                uoemod->uoexec.ValueStack.top().set( new BObject( str ) );
            }
        }
    }
    uoemod->uoexec.os_module->revive();
    uoemod->prompt_chr = NULL;
    client->gd->prompt_uoemod = NULL;
}
MESSAGE_HANDLER_VARLEN( PKTBI_9A, handle_prompt );

BObjectImp* UOExecutorModule::mf_PromptInput()
{
    Character* chr;
    Item* item;
    const String* prompt;
    if (!getCharacterParam( exec, 0, chr ) ||
        !getItemParam( exec, 1, item ) ||
        !exec.getStringParam( 2, prompt ))
    {
        return new BError( "Invalid parameter" );
    }

    if (chr->client == NULL)
    {
        return new BError( "No client attached" );
    }

    if (chr->has_active_prompt() != false)
    {
        return new BError( "Another script has an active prompt" );
    }

    send_sysmessage( chr->client, prompt->data() );

    chr->client->gd->prompt_uoemod = this;
    prompt_chr = chr;

    send_prompt( chr->client, ctBEu32( item->serial ) );
    uoexec.os_module->suspend();
    return new BLong(0);
}
