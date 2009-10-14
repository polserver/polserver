#include "clib/stl_inc.h"
/*

#include "clib/endian.h"

#include "charactr.h"
#include "client.h"
#include "item.h"
#include "msghandl.h"
#include "sockio.h"
#include "target.h"
#include "ufunc.h"
#include "umsgtype.h"

#include "dye.h"

void handle_dye( Client* client, MSG95_DYE_VAT* msg )
{
    u32 serial = cfBEu32( msg->serial );
    Item* dye_tub = find_legal_item( client->chr, serial );
    if (dye_tub == NULL)
    {
        send_sysmessage( client, "Couldn't find that!" );
        return;
    }
    if (dye_tub->objtype_ != UOBJ_DYE_TUB)
    {
        send_sysmessage( client, "Dyes can only be used on dye tubs." );
        return;
    }

    dye_tub->color = cfBEu16(msg->graphic_or_color); 
    dye_tub->color &= VALID_ITEM_COLOR_MASK;
    dye_tub->color_ext = ctBEu16( dye_tub->color );

    update_item_to_inrange( dye_tub );
}
MESSAGE_HANDLER( MSG95_DYE_VAT, handle_dye );

void handle_dyes_cursor( Character* chr, UObject* targetted )
{
    if (chr->client != NULL)
    {
        if (targetted->objtype_ != UOBJ_DYE_TUB)
        {
            send_sysmessage( chr->client, "You can't use dyes on that." );
            return;
        }

        MSG95_DYE_VAT msg;
        msg.msgtype = MSG95_DYE_VAT_ID;
        msg.serial = targetted->serial_ext;
        msg.unk = 0;
        msg.graphic_or_color = targetted->graphic_ext;

        transmit( chr->client, &msg, sizeof msg );
    }
}

LosCheckedTargetCursor dyes_cursor( handle_dyes_cursor );
void Dyes::builtin_on_use( Client* client )
{
    send_sysmessage( client, "Select a dye tub to use the dyes on." );
    dyes_cursor.send_object_cursor( client );
}

void handle_dyetub_cursor( Character* chr, UObject* obj )
{
    if (chr->dye_tub != NULL)
    {
        if (IsItem(obj->serial))
        {
            Item* item = static_cast<Item*>(obj);

            if (chr->can_dye( item ))
            {
                item->color = chr->dye_tub->color;
                item->color_ext = chr->dye_tub->color_ext;
                
                update_item_to_inrange( item );
            }
        }
        chr->dye_tub = NULL;
    }
}
LosCheckedTargetCursor dyetub_cursor( handle_dyetub_cursor );

void DyeTub::builtin_on_use( Client* client )
{
    client->chr->dye_tub = this;
    send_sysmessage( client, "Select something to dye." );
    dyetub_cursor.send_object_cursor( client );
}

*/
