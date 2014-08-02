
#include "../../clib/logfacility.h"

#include "packetdefs.h"

namespace Pol {
  namespace Network {

    void SendWorldItem::Send( Client* client )
    {
      if ( client->ClientType & CLIENTTYPE_7000 )
      {
        if ( _p->offset==1)
          buildF3();
        if ( client->ClientType & CLIENTTYPE_7090 ) /*once known split class?*/
          _p.Send( client, 26 );
        else
          _p.Send( client, 24 );
      }
      else
      {
        if ( _p_old->offset == 1 )
          build1A();
        _p_old.Send( client, _p_oldlen );
      }
    }

    void SendWorldItem::updateFlags( u8 flags )
    {
      if ( flags != _flags )
      {
        _flags = flags;
        if ( _p_old->offset != 1 )
        {
          _p_old->offset = _p_oldlen - 1;
          _p_old->Write<u8>( _flags );
        }
        if ( _p->offset != 1 )
        {
          _p->offset = 23;
          _p->Write<u8>( _flags );
        }
      }
    }

    void SendWorldItem::build1A()
    {
      _p_old->offset = 1;
      // transmit item info
      _p_old->offset += 2;
      // If the 0x80000000 is left out, the item won't show up. 
      _p_old->WriteFlipped<u32>( static_cast<u32>( 0x80000000 | _serial ) ); // bit 0x80000000 enables piles
      _p_old->WriteFlipped<u16>( _graphic );
      _p_old->WriteFlipped<u16>( _amount );
      if ( _facing == 0 )
      {
        _p_old->WriteFlipped<u16>( _x );
        // bits 0x80 and 0x40 are Dye and Move (dunno which is which)
        _p_old->WriteFlipped<u16>( static_cast<u16>( 0xC000 | _y ) ); // dyeable and moveable?
      }
      else
      {
        _p_old->WriteFlipped<u16>( static_cast<u16>( 0x8000 | _x ) );
        // bits 0x80 and 0x40 are Dye and Move (dunno which is which)
        _p_old->WriteFlipped<u16>( static_cast<u16>( 0xC000 | _y ) ); // dyeable and moveable?
        _p_old->Write<u8>( _facing );
      }
      _p_old->Write<s8>( _z );
      _p_old->WriteFlipped<u16>( _color );
      _p_old->Write<u8>( _flags );
      _p_oldlen = _p_old->offset;
      _p_old->offset = 1;
      _p_old->WriteFlipped<u16>( _p_oldlen );
    }
    void SendWorldItem::buildF3( )
    {
      _p->offset = 1;
      _p->WriteFlipped<u16>( 0x1 );
      _p->offset++; // datatype
      _p->WriteFlipped<u32>( _serial );
      _p->WriteFlipped<u16>( _graphic );
      _p->Write<u8>( 0 );
      _p->WriteFlipped<u16>( _amount );
      _p->WriteFlipped<u16>( _amount );
      _p->WriteFlipped<u16>( _x );
      _p->WriteFlipped<u16>( _y );
      _p->Write<s8>( _z );
      _p->Write<u8>( _facing );
      _p->WriteFlipped<u16>( _color );
      _p->Write<u8>( _flags );
    }

    SendWorldItem::SendWorldItem( u32 serial, u16 graphic, u16 amount, u16 x, u16 y, s8 z, u8 facing, u16 color, u8 flags )
      : PktSender(),
      _p_oldlen( 0 ),
      _serial(serial),
      _graphic(graphic),
      _amount(amount),
      _x(x),
      _y(y),
      _z(z),
      _facing(facing),
      _color(color),
      _flags(flags)
    {
    }

    SendWorldMulti::SendWorldMulti( u32 serial_ext, u16 graphic, u16 x, u16 y, s8 z, u16 color )
      : PktSender(),
      _p_oldlen( 0 ),
      _serial_ext( serial_ext ),
      _graphic( graphic ),
      _x( x ),
      _y( y ),
      _z( z ),
      _color( color )
    {
      
    }

    void SendWorldMulti::build1A( )
    {
      _p_old->offset = 1;
      _p_old->offset += 2;
      _p_old->Write<u32>( _serial_ext );
      _p_old->WriteFlipped<u16>( _graphic | 0x4000 );
      _p_old->WriteFlipped<u16>( _x );
      _p_old->WriteFlipped<u16>( _y );
      _p_old->Write<s8>( _z );
      _p_oldlen = _p_old->offset;
      _p_old->offset = 1;
      _p_old->WriteFlipped<u16>( _p_oldlen );
    }

    void SendWorldMulti::buildF3( )
    {
      _p->offset = 1;
      _p->WriteFlipped<u16>( 0x1 );
      _p->Write<u8>( static_cast<u8>( 0x02 ) );
      _p->Write<u32>( _serial_ext );
      _p->WriteFlipped<u16>( _graphic );
      _p->Write<u8>( 0 );
      _p->WriteFlipped<u16>( static_cast<u16>( 0x1 ) ); //amount
      _p->WriteFlipped<u16>( static_cast<u16>( 0x1 ) ); //amount
      _p->WriteFlipped<u16>( _x );
      _p->WriteFlipped<u16>( _y );
      _p->Write<s8>( _z );
      _p->Write<u8>( 0 ); // facing
      _p->WriteFlipped<u16>( _color );
      _p->Write<u8>( 0 ); // flags
    }

    void SendWorldMulti::Send( Client* client )
    {
      if ( client->ClientType & CLIENTTYPE_7000 )
      {
        if ( _p->offset == 1 )
          buildF3();
        if ( client->ClientType & CLIENTTYPE_7090 ) /*once known split class?*/
          _p.Send( client, 26 );
        else
          _p.Send( client, 24 );
      }
      else
      {
        if ( _p_old->offset == 1 )
          build1A();
        _p_old.Send( client, _p_oldlen );
      }
    }


    AddItemContainerMsg::AddItemContainerMsg( u32 serial_ext, u16 graphic, u16 amount, u16 x, u16 y, u8 slotindex, u32 containerserial_ext, u16 color )
      : PktSender(),
      _serial_ext( serial_ext ),
      _graphic( graphic ),
      _amount( amount ),
      _x( x ),
      _y( y ),
      _slotindex( slotindex ),
      _containerserial_ext( containerserial_ext ),
      _color( color )
    {}

    void AddItemContainerMsg::buildLegacy()
    {
      _p_old->offset = 1;
      _p_old->Write<u32>( _serial_ext );
      _p_old->WriteFlipped<u16>( _graphic );
      _p_old->offset++; //unk7 layer?
      _p_old->WriteFlipped<u16>( _amount );
      _p_old->WriteFlipped<u16>( _x );
      _p_old->WriteFlipped<u16>( _y );
      _p_old->Write<u32>( _containerserial_ext );
      _p_old->WriteFlipped<u16>( _color );
    }
    void AddItemContainerMsg::build()
    {
      _p->offset = 1;
      _p->Write<u32>( _serial_ext );
      _p->WriteFlipped<u16>( _graphic );
      _p->offset++; //unk7 layer?
      _p->WriteFlipped<u16>( _amount );
      _p->WriteFlipped<u16>( _x );
      _p->WriteFlipped<u16>( _y );
      _p->Write<u8>( _slotindex );
      _p->Write<u32>( _containerserial_ext );
      _p->WriteFlipped<u16>( _color );
    }
    
    void AddItemContainerMsg::Send( Client* client )
    {
      if ( client->ClientType & CLIENTTYPE_6017 )
      {
        if ( _p->offset == 1 )
          build();
        _p.Send( client, 21 );
      }
      else
      {
        if ( _p_old->offset == 1 )
          buildLegacy();
        _p_old.Send( client, 20 );
      }
    }
  }
}