
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
      _p_old->WriteFlipped<u32>( 0x80000000u | _serial ); // bit 0x80000000 enables piles
      _p_old->WriteFlipped<u16>( _graphic );
      _p_old->WriteFlipped<u16>( _amount );
      if ( _facing == 0 )
      {
        _p_old->WriteFlipped<u16>( _x );
        // bits 0x80 and 0x40 are Dye and Move (dunno which is which)
        _p_old->WriteFlipped<u16>( 0xC000u | _y ); // dyeable and moveable?
      }
      else
      {
        _p_old->WriteFlipped<u16>( 0x8000u | _x );
        // bits 0x80 and 0x40 are Dye and Move (dunno which is which)
        _p_old->WriteFlipped<u16>( 0xC000u | _y ); // dyeable and moveable?
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
      _p->WriteFlipped<u16>( 0x1u );
      _p->offset++; // datatype
      _p->WriteFlipped<u32>( _serial );
      _p->WriteFlipped<u16>( _graphic );
      _p->Write<u8>( 0u );
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
      _flags(flags),
      _p_old( ),
      _p( )
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
      _color( color ),
      _p_old(),
      _p()
    {
      
    }

    void SendWorldMulti::build1A( )
    {
      _p_old->offset = 1;
      _p_old->offset += 2;
      _p_old->Write<u32>( _serial_ext );
      _p_old->WriteFlipped<u16>( _graphic | 0x4000u );
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
      _p->WriteFlipped<u16>( 0x1u );
      _p->Write<u8>( 0x02u );
      _p->Write<u32>( _serial_ext );
      _p->WriteFlipped<u16>( _graphic );
      _p->Write<u8>( 0u );
      _p->WriteFlipped<u16>( 0x1u ); //amount
      _p->WriteFlipped<u16>( 0x1u ); //amount
      _p->WriteFlipped<u16>( _x );
      _p->WriteFlipped<u16>( _y );
      _p->Write<s8>( _z );
      _p->Write<u8>( 0u ); // facing
      _p->WriteFlipped<u16>( _color );
      _p->Write<u8>( 0u ); // flags
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
      _color( color ),
      _p_old(),
      _p()
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
        _p.Send( client, _p->SIZE );
      }
      else
      {
        if ( _p_old->offset == 1 )
          buildLegacy();
        _p_old.Send( client, _p->SIZE-1 );
      }
    }

    MobileAnimationMsg::MobileAnimationMsg( u32 serial_ext )
      :PktSender(),
      _serial_ext( serial_ext ),
      _anim( 0 ),
      _action( 0 ),
      _subaction( 0 ),
      _action_old( 0 ),
      _framecount_old( 0 ),
      _repeat_old( 0 ),
      _backward_old( 0 ),
      _repeat_flag_old( 0 ),
      _delay_old( 0 ),
      _oldanim_valid( false ),
      _newanim_valid( false ),
      _p_old( ),
      _p( )
    {}
    MobileAnimationMsg::MobileAnimationMsg( u32 serial_ext, u16 anim, u16 action, u8 subaction,
                                            u16 action_old, u16 framecount_old, u16 repeat_old,
                                            DIRECTION_FLAG_OLD backward_old, REPEAT_FLAG_OLD repeat_flag_old, u8 delay_old,
                                            bool oldanim_valid, bool newanim_valid )
        : PktSender(),
        _serial_ext( serial_ext ),
        _anim( anim ),
        _action( action ),
        _subaction( subaction ),
        _action_old( action_old ),
        _framecount_old( framecount_old ),
        _repeat_old( repeat_old ),
        _backward_old( static_cast<u8>( backward_old ) ),
        _repeat_flag_old( static_cast<u8>( repeat_flag_old ) ),
        _delay_old( delay_old ),
        _oldanim_valid( oldanim_valid ),
        _newanim_valid( newanim_valid ),
        _p_old(),
        _p()
    {}

    void MobileAnimationMsg::update( u16 anim, u16 action, u8 subaction,
                                     u16 action_old, u16 framecount_old, u16 repeat_old,
                                     DIRECTION_FLAG_OLD backward_old, REPEAT_FLAG_OLD repeat_flag_old, u8 delay_old,
                                     bool oldanim_valid, bool newanim_valid )
    {
      _anim = anim;
      _action = action;
      _subaction = subaction;
      _action_old = action_old;
      _framecount_old = framecount_old;
      _repeat_old = repeat_old;
      _backward_old = static_cast<u8>( backward_old );
      _repeat_flag_old = static_cast<u8>( repeat_flag_old );
      _delay_old = delay_old;
      _oldanim_valid = oldanim_valid;
      _newanim_valid = newanim_valid;
      if ( _oldanim_valid && _p_old->offset != 1 )
        build6E();
      if ( _newanim_valid && _p->offset != 1 )
        build();
    }
    void MobileAnimationMsg::build()
    {
      _p->offset = 1;
      _p->Write<u32>( _serial_ext );
      _p->WriteFlipped<u16>( _anim );
      _p->WriteFlipped<u16>( _action );
      _p->Write<u8>( _subaction );
    }

    void MobileAnimationMsg::build6E()
    {
      _p_old->offset = 1;
      _p_old->Write<u32>( _serial_ext );
      _p_old->WriteFlipped<u16>( _action_old );
      _p_old->WriteFlipped<u16>( _framecount_old );
      _p_old->WriteFlipped<u16>( _repeat_old );
      _p_old->Write<u8>( _backward_old );
      _p_old->Write<u8>( _repeat_flag_old );
      _p_old->Write<u8>( _delay_old );
    }

    void MobileAnimationMsg::Send( Client* client )
    {
      if ( client->ClientType & CLIENTTYPE_7090 )
      {
        if ( !_newanim_valid )
          return;
        if ( _p->offset == 1 )
          build();
        _p.Send( client, _p->SIZE );
      }
      else
      {
        if ( !_oldanim_valid )
          return;
        if ( _p_old->offset == 1 )
          build6E();
        _p_old.Send( client, _p_old->SIZE );
      }
    }


	PlaySoundPkt::PlaySoundPkt(u8 type, u16 effect, u16 xcenter, u16 ycenter, s16 zcenter)
	  : _type(type),
	  _effect(effect),
	  _xcenter(xcenter),
	  _ycenter(ycenter),
	  _zcenter(zcenter),
	  _p()
	{
	}

	void PlaySoundPkt::Send( Client* client )
	{
	  if ( _p->offset == 1 )
        build();
      _p.Send( client, _p->SIZE );
	}

	void PlaySoundPkt::build()
	{
	  _p->offset = 1;
	  _p->Write<u8>( _type );
	  _p->WriteFlipped<u16>( _effect );
	  _p->offset += 2; //volume
	  _p->WriteFlipped<u16>( _xcenter );
	  _p->WriteFlipped<u16>( _ycenter );
	  _p->WriteFlipped<s16>( _zcenter );
	}

	RemoveObjectPkt::RemoveObjectPkt(u32 serial_ext)
	  : _serial(serial_ext),
	  _p()
	{
	}

	void RemoveObjectPkt::update( u32 serial )
	{
	  _serial = serial;
	  if (_p->offset != 1)
		build();
	}

	void RemoveObjectPkt::Send( Client* client )
	{
	  if ( _p->offset == 1 )
        build();
      _p.Send( client, _p->SIZE );
	}

	void RemoveObjectPkt::build()
	{
	  _p->offset = 1;
      _p->Write<u32>( _serial );
	}

  }
}