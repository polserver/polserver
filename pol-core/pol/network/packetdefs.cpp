
#include "packetdefs.h"

#include "../../clib/rawtypes.h"
#include "../globals/settings.h"
#include "../mobile/charactr.h"
#include "../uobject.h"
#include "client.h"
#include "pktdef.h"


namespace Pol::Network
{
void SendWorldItem::Send( Client* client )
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
  _p_old->WriteFlipped<u32>( 0x80000000u | _serial );  // bit 0x80000000 enables piles
  _p_old->WriteFlipped<u16>( _graphic );
  _p_old->WriteFlipped<u16>( _amount );
  if ( _facing == 0 )
  {
    _p_old->WriteFlipped<u16>( _pos.x() );
    // bits 0x80 and 0x40 are Dye and Move (dunno which is which)
    _p_old->WriteFlipped<u16>( 0xC000u | _pos.y() );  // dyeable and moveable?
  }
  else
  {
    _p_old->WriteFlipped<u16>( 0x8000u | _pos.x() );
    // bits 0x80 and 0x40 are Dye and Move (dunno which is which)
    _p_old->WriteFlipped<u16>( 0xC000u | _pos.y() );  // dyeable and moveable?
    _p_old->Write<u8>( _facing );
  }
  _p_old->Write<s8>( _pos.z() );
  _p_old->WriteFlipped<u16>( _color );
  _p_old->Write<u8>( _flags );
  _p_oldlen = _p_old->offset;
  _p_old->offset = 1;
  _p_old->WriteFlipped<u16>( _p_oldlen );
}
void SendWorldItem::buildF3()
{
  _p->offset = 1;
  _p->WriteFlipped<u16>( 0x1u );
  _p->offset++;  // datatype
  _p->WriteFlipped<u32>( _serial );
  _p->WriteFlipped<u16>( _graphic );
  _p->Write<u8>( 0u );
  _p->WriteFlipped<u16>( _amount );
  _p->WriteFlipped<u16>( _amount );
  _p->WriteFlipped<u16>( _pos.x() );
  _p->WriteFlipped<u16>( _pos.y() );
  _p->Write<s8>( _pos.z() );
  _p->Write<u8>( _facing );
  _p->WriteFlipped<u16>( _color );
  _p->Write<u8>( _flags );
}

SendWorldItem::SendWorldItem( u32 serial, u16 graphic, u16 amount, Core::Pos3d pos, u8 facing,
                              u16 color, u8 flags )
    : PktSender(),
      _p_oldlen( 0 ),
      _serial( serial ),
      _graphic( graphic ),
      _amount( amount ),
      _pos( std::move( pos ) ),
      _facing( facing ),
      _color( color ),
      _flags( flags ),
      _p_old(),
      _p()
{
}

SendWorldMulti::SendWorldMulti( u32 serial_ext, u16 graphic, Core::Pos3d pos, u16 color )
    : PktSender(),
      _p_oldlen( 0 ),
      _serial_ext( serial_ext ),
      _graphic( graphic ),
      _pos( std::move( pos ) ),
      _color( color ),
      _p_old(),
      _p()
{
}

void SendWorldMulti::build1A()
{
  _p_old->offset = 1;
  _p_old->offset += 2;
  _p_old->Write<u32>( _serial_ext );
  _p_old->WriteFlipped<u16>( _graphic | 0x4000u );
  _p_old->WriteFlipped<u16>( _pos.x() );
  _p_old->WriteFlipped<u16>( _pos.y() );
  _p_old->Write<s8>( _pos.z() );
  _p_oldlen = _p_old->offset;
  _p_old->offset = 1;
  _p_old->WriteFlipped<u16>( _p_oldlen );
}

void SendWorldMulti::buildF3()
{
  _p->offset = 1;
  _p->WriteFlipped<u16>( 0x1u );
  _p->Write<u8>( 0x02u );
  _p->Write<u32>( _serial_ext );
  _p->WriteFlipped<u16>( _graphic );
  _p->Write<u8>( 0u );
  _p->WriteFlipped<u16>( 0x1u );  // amount
  _p->WriteFlipped<u16>( 0x1u );  // amount
  _p->WriteFlipped<u16>( _pos.x() );
  _p->WriteFlipped<u16>( _pos.y() );
  _p->Write<s8>( _pos.z() );
  _p->Write<u8>( 0u );  // facing
  _p->WriteFlipped<u16>( _color );
  _p->Write<u8>( 0u );  // flags
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


AddItemContainerMsg::AddItemContainerMsg( u32 serial_ext, u16 graphic, u16 amount, Core::Pos2d pos,
                                          u8 slotindex, u32 containerserial_ext, u16 color )
    : PktSender(),
      _serial_ext( serial_ext ),
      _graphic( graphic ),
      _amount( amount ),
      _pos( std::move( pos ) ),
      _slotindex( slotindex ),
      _containerserial_ext( containerserial_ext ),
      _color( color ),
      _p_old(),
      _p()
{
}

void AddItemContainerMsg::buildLegacy()
{
  _p_old->offset = 1;
  _p_old->Write<u32>( _serial_ext );
  _p_old->WriteFlipped<u16>( _graphic );
  _p_old->offset++;  // unk7 layer?
  _p_old->WriteFlipped<u16>( _amount );
  _p_old->WriteFlipped<u16>( _pos.x() );
  _p_old->WriteFlipped<u16>( _pos.y() );
  _p_old->Write<u32>( _containerserial_ext );
  _p_old->WriteFlipped<u16>( _color );
}
void AddItemContainerMsg::build()
{
  _p->offset = 1;
  _p->Write<u32>( _serial_ext );
  _p->WriteFlipped<u16>( _graphic );
  _p->offset++;  // unk7 layer?
  _p->WriteFlipped<u16>( _amount );
  _p->WriteFlipped<u16>( _pos.x() );
  _p->WriteFlipped<u16>( _pos.y() );
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
    _p.Send( client, _p->getSize() );
  }
  else
  {
    if ( _p_old->offset == 1 )
      buildLegacy();
    _p_old.Send( client, _p->getSize() - 1 );
  }
}

MobileAnimationMsg::MobileAnimationMsg( u32 serial_ext )
    : PktSender(),
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
      _p_old(),
      _p()
{
}
MobileAnimationMsg::MobileAnimationMsg( u32 serial_ext, u16 anim, u16 action, u8 subaction,
                                        u16 action_old, u16 framecount_old, u16 repeat_old,
                                        Core::DIRECTION_FLAG_OLD backward_old,
                                        Core::REPEAT_FLAG_OLD repeat_flag_old, u8 delay_old,
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
{
}

void MobileAnimationMsg::update( u16 anim, u16 action, u8 subaction, u16 action_old,
                                 u16 framecount_old, u16 repeat_old,
                                 Core::DIRECTION_FLAG_OLD backward_old,
                                 Core::REPEAT_FLAG_OLD repeat_flag_old, u8 delay_old,
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
    _p.Send( client, _p->getSize() );
  }
  else
  {
    if ( !_oldanim_valid )
      return;
    if ( _p_old->offset == 1 )
      build6E();
    _p_old.Send( client, _p_old->getSize() );
  }
}


PlaySoundPkt::PlaySoundPkt( u8 type, u16 effect, Core::Pos3d center )
    : PktSender(), _type( type ), _effect( effect ), _center( std::move( center ) ), _p()
{
}

void PlaySoundPkt::Send( Client* client )
{
  if ( _p->offset == 1 )
    build();
  _p.Send( client, _p->getSize() );
}

void PlaySoundPkt::build()
{
  _p->offset = 1;
  _p->Write<u8>( _type );
  _p->WriteFlipped<u16>( _effect );
  _p->offset += 2;  // volume
  _p->WriteFlipped<u16>( _center.x() );
  _p->WriteFlipped<u16>( _center.y() );
  _p->WriteFlipped<s16>( _center.z() );
}

RemoveObjectPkt::RemoveObjectPkt( u32 serial_ext ) : _serial( serial_ext ), _p() {}

void RemoveObjectPkt::update( u32 serial )
{
  _serial = serial;
  if ( _p->offset != 1 )
    build();
}

void RemoveObjectPkt::Send( Client* client )
{
  if ( _p->offset == 1 )
    build();
  _p.Send( client, _p->getSize() );
}

void RemoveObjectPkt::build()
{
  _p->offset = 1;
  _p->Write<u32>( _serial );
}

SendDamagePkt::SendDamagePkt( u32 serial_ext, u16 damage )
    : PktSender(), _serial( serial_ext ), _damage( damage ), _p_old(), _p()
{
}
void SendDamagePkt::Send( Client* client )
{
  if ( client->ClientType & CLIENTTYPE_4070 )
  {
    if ( _p->offset == 1 )
      build();
    _p.Send( client, _p->getSize() );
  }
  else
  {
    if ( _p_old->offset == 1 )
      buildold();
    _p_old.Send( client );
  }
}

void SendDamagePkt::build()
{
  _p->offset = 1;
  _p->Write<u32>( _serial );
  _p->WriteFlipped<u16>( _damage );
}
void SendDamagePkt::buildold()
{
  _p_old->offset = 1;
  _p_old->WriteFlipped<u16>( 11u );
  _p_old->offset += 2;  // sub
  _p_old->Write<u8>( 1u );
  _p_old->Write<u32>( _serial );
  if ( _damage > 0xFF )
    _p_old->Write<u8>( 0xFFu );
  else
    _p_old->Write<u8>( _damage );
}


ObjRevisionPkt::ObjRevisionPkt( u32 serial_ext, u32 rev )
    : PktSender(), _serial_ext( serial_ext ), _rev( rev ), _p_old(), _p()
{
}

void ObjRevisionPkt::Send( Client* client )
{
  if ( !client->acctSupports( Plib::ExpansionVersion::AOS ) )
    return;
  if ( !Core::settingsManager.ssopt.features.supportsAOS() )
    return;
  if ( ( Core::settingsManager.ssopt.force_new_objcache_packets ) ||
       ( client->ClientType & Network::CLIENTTYPE_5000 ) )
  {
    if ( _p->offset == 1 )
      build();
    _p.Send( client, _p->getSize() );
  }
  else
  {
    if ( _p_old->offset == 1 )
      buildold();
    _p_old.Send( client );
  }
}

void ObjRevisionPkt::build()
{
  _p->offset = 1;
  _p->Write<u32>( _serial_ext );
  _p->WriteFlipped<u32>( _rev );
}
void ObjRevisionPkt::buildold()
{
  _p_old->offset = 1;
  _p_old->WriteFlipped<u16>( 0xDu );
  _p_old->offset += 2;  // sub
  _p_old->Write<u32>( _serial_ext );
  _p_old->WriteFlipped<u32>( _rev );
}

GraphicEffectPkt::GraphicEffectPkt()
    : PktSender(),
      _effect_type( 0 ),
      _src_serial_ext( 0 ),
      _dst_serial_ext( 0 ),
      _effect( 0 ),
      _src(),
      _dst(),
      _speed( 0 ),
      _loop( 0 ),
      _explode( 0 ),
      _unk26( 0 ),
      _p()
{
}

void GraphicEffectPkt::movingEffect( const Core::UObject* src, const Core::UObject* dst, u16 effect,
                                     u8 speed, u8 loop, u8 explode )
{
  _effect_type = Core::PKTOUT_C0::EFFECT_MOVING;
  _src_serial_ext = src->serial_ext;
  _dst_serial_ext = dst->serial_ext;
  _effect = effect;
  _src = src->toplevel_pos().xyz() + Core::Vec3d( 0, 0, src->height );
  _dst = dst->toplevel_pos().xyz() + Core::Vec3d( 0, 0, dst->height );
  _speed = speed;
  _loop = loop;
  _explode = explode;
}
void GraphicEffectPkt::movingEffect( Core::Pos3d src, Core::Pos3d dst, u16 effect, u8 speed,
                                     u8 loop, u8 explode )
{
  _effect_type = Core::PKTOUT_C0::EFFECT_MOVING;
  _effect = effect;
  _src = std::move( src );
  _dst = std::move( dst );
  _speed = speed;
  _loop = loop;
  _explode = explode;
}
void GraphicEffectPkt::lightningBold( const Core::UObject* center )
{
  _effect_type = Core::PKTOUT_C0::EFFECT_LIGHTNING;
  _src_serial_ext = center->serial_ext;
  _src = center->toplevel_pos().xyz();
}

void GraphicEffectPkt::followEffect( const Core::UObject* center, u16 effect, u8 speed, u8 loop )
{
  _effect_type = Core::PKTOUT_C0::EFFECT_FIXEDFROM;
  _src_serial_ext = center->serial_ext;
  _effect = effect;
  _src = center->toplevel_pos().xyz();
  _speed = speed;
  _loop = loop;
}

void GraphicEffectPkt::stationaryEffect( Core::Pos3d src, u16 effect, u8 speed, u8 loop,
                                         u8 explode )
{
  _effect_type = Core::PKTOUT_C0::EFFECT_FIXEDXYZ;
  _effect = effect;
  _src = std::move( src );
  _speed = speed;
  _loop = loop;
  _explode = explode;
  _unk26 = 1;  // this is right for teleport, anyway
}

void GraphicEffectPkt::build()
{
  _p->offset = 1;
  _p->Write<u8>( _effect_type );
  _p->Write<u32>( _src_serial_ext );
  _p->Write<u32>( _dst_serial_ext );
  _p->WriteFlipped<u16>( _effect );
  _p->WriteFlipped<u16>( _src.x() );
  _p->WriteFlipped<u16>( _src.y() );
  _p->Write<s8>( _src.z() );
  _p->WriteFlipped<u16>( _dst.x() );
  _p->WriteFlipped<u16>( _dst.y() );
  _p->Write<s8>( _dst.z() );
  _p->Write<u8>( _speed );
  _p->Write<u8>( _loop );
  _p->offset += 2;  // unk24,unk25
  _p->Write<u8>( _unk26 );
  _p->Write<u8>( _explode );
}

void GraphicEffectPkt::Send( Client* client )
{
  if ( _p->offset == 1 )
    build();
  _p.Send( client, _p->getSize() );
}


GraphicEffectExPkt::GraphicEffectExPkt()
    : PktSender(),
      _effect_type( 0 ),
      _src_serial_ext( 0 ),
      _dst_serial_ext( 0 ),
      _effect( 0 ),
      _src(),
      _dst(),
      _speed( 0 ),
      _duration( 0 ),
      _direction( 0 ),
      _explode( 0 ),
      _hue( 0 ),
      _render( 0 ),
      _effect3d( 0 ),
      _effect3dexplode( 0 ),
      _effect3dsound( 0 ),
      _itemid( 0 ),
      _layer( 0 ),
      _p()
{
}

void GraphicEffectExPkt::movingEffect( const Core::UObject* src, const Core::UObject* dst,
                                       u16 effect, u8 speed, u8 duration, u32 hue, u32 render,
                                       u8 direction, u8 explode, u16 effect3d, u16 effect3dexplode,
                                       u16 effect3dsound )
{
  _effect_type = Core::PKTOUT_C0::EFFECT_MOVING;
  _src_serial_ext = src->serial_ext;
  _dst_serial_ext = dst->serial_ext;
  _effect = effect;
  _src = src->toplevel_pos().xyz() + Core::Vec3d( 0, 0, src->height );
  _dst = dst->toplevel_pos().xyz() + Core::Vec3d( 0, 0, dst->height );
  _speed = speed;
  _duration = duration;
  _direction = direction;
  _explode = explode;
  _hue = hue;
  _render = render;
  _effect3d = effect3d;
  _effect3dexplode = effect3dexplode;
  _effect3dsound = effect3dsound;
  _layer = 0xFF;
}
void GraphicEffectExPkt::movingEffect( Core::Pos3d src, Core::Pos3d dst, u16 effect, u8 speed,
                                       u8 duration, u32 hue, u32 render, u8 direction, u8 explode,
                                       u16 effect3d, u16 effect3dexplode, u16 effect3dsound )
{
  _effect_type = Core::PKTOUT_C0::EFFECT_MOVING;
  _effect = effect;
  _src = std::move( src );
  _dst = std::move( dst );
  _speed = speed;
  _duration = duration;
  _direction = direction;
  _explode = explode;
  _hue = hue;
  _render = render;
  _effect3d = effect3d;
  _effect3dexplode = effect3dexplode;
  _effect3dsound = effect3dsound;
  _layer = 0xFF;
}

void GraphicEffectExPkt::followEffect( const Core::UObject* center, u16 effect, u8 speed,
                                       u8 duration, u32 hue, u32 render, u8 layer, u16 effect3d )
{
  _effect_type = Core::PKTOUT_C0::EFFECT_FIXEDFROM;
  _src_serial_ext = center->serial_ext;
  _dst_serial_ext = center->serial_ext;
  _effect = effect;
  _src = center->toplevel_pos().xyz();
  _dst = center->toplevel_pos().xyz();
  _speed = speed;
  _duration = duration;
  _direction = 1;
  _hue = hue;
  _render = render;
  _effect3d = effect3d;
  _effect3dexplode = 1;
  _itemid = center->serial_ext;
  _layer = layer;
}

void GraphicEffectExPkt::stationaryEffect( Core::Pos3d pos, u16 effect, u8 speed, u8 duration,
                                           u32 hue, u32 render, u16 effect3d )
{
  _effect_type = Core::PKTOUT_C0::EFFECT_FIXEDXYZ;
  _effect = effect;
  _src = pos;
  _dst = std::move( pos );
  _speed = speed;
  _duration = duration;
  _direction = 1;
  _hue = hue;
  _render = render;
  _effect3d = effect3d;
  _effect3dexplode = 1;
  _layer = 0xFF;
}

void GraphicEffectExPkt::build()
{
  // C0 part
  _p->offset = 1;
  _p->Write<u8>( _effect_type );
  _p->Write<u32>( _src_serial_ext );
  _p->Write<u32>( _dst_serial_ext );
  _p->WriteFlipped<u16>( _effect );
  _p->WriteFlipped<u16>( _src.x() );
  _p->WriteFlipped<u16>( _src.y() );
  _p->Write<s8>( _src.z() );
  _p->WriteFlipped<u16>( _dst.x() );
  _p->WriteFlipped<u16>( _dst.y() );
  _p->Write<s8>( _dst.z() );
  _p->Write<u8>( _speed );
  _p->Write<u8>( _duration );
  _p->offset += 2;  // u16 unk
  _p->Write<u8>( _direction );
  _p->Write<u8>( _explode );
  _p->WriteFlipped<u32>( _hue );
  _p->WriteFlipped<u32>( _render );
  // C7 part
  _p->WriteFlipped<u16>( _effect3d );         // see particleffect subdir
  _p->WriteFlipped<u16>( _effect3dexplode );  // 0 if no explosion
  _p->WriteFlipped<u16>( _effect3dsound );    // for moving effects, 0 otherwise
  _p->Write<u32>( _itemid );                  // if target is item (type 2), 0 otherwise
  _p->Write<u8>( _layer );  //(of the character, e.g left hand, right hand,  0-5,7, 0xff: moving
  // effect or target is no char)
  _p->offset += 2;  // u16 unk_effect
}

void GraphicEffectExPkt::Send( Client* client )
{
  if ( _p->offset == 1 )
    build();
  _p.Send( client, _p->getSize() );
}


HealthBarStatusUpdate::HealthBarStatusUpdate( u32 serial_ext, Color color, bool enable )
    : PktSender(), _serial_ext( serial_ext ), _enable( enable ), _color( color ), _p()
{
}
void HealthBarStatusUpdate::build()
{
  _p->offset = 1;
  _p->WriteFlipped<u16>( _p->getSize() );
  _p->Write<u32>( _serial_ext );
  _p->WriteFlipped<u16>( 1u );  // unk
  _p->WriteFlipped<u16>( _color );
  _p->Write<u8>( _enable ? 1u : 0u );  // flag
}
void HealthBarStatusUpdate::Send( Client* client )
{
  if ( client->ClientType & CLIENTTYPE_UOKR )
  {
    if ( _p->offset == 1 )
      build();
    _p.Send( client );
  }
}


MoveChrPkt::MoveChrPkt( const Mobile::Character* chr ) : PktSender(), _chr( chr ), _p() {}

void MoveChrPkt::build()
{
  _p->offset = 1;
  _p->Write<u32>( _chr->serial_ext );
  _p->WriteFlipped<u16>( _chr->graphic );
  _p->WriteFlipped<u16>( _chr->x() );
  _p->WriteFlipped<u16>( _chr->y() );
  _p->Write<s8>( _chr->z() );
  _p->Write<u8>( ( _chr->dir & 0x80u ) |
                 _chr->facing );  // NOTE, this only includes mask 0x07 of the last MOVE message
  _p->WriteFlipped<u16>( _chr->color );
}
void MoveChrPkt::Send( Client* client )
{
  if ( _p->offset == 1 )
    build();
  _p->offset = 15;
  _p->Write<u8>( _chr->get_flag1( client ) );
  _p->Write<u8>( _chr->hilite_color_idx( client->chr ) );
  _p.Send( client );
}
}  // namespace Pol::Network
