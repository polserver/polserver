
#ifndef POL_PACKETDEFS_H
#define POL_PACKETDEFS_H

#include "../../clib/rawtypes.h"
#include "../action.h"
#include "base/position.h"
#include "packethelper.h"
#include "packets.h"

namespace Pol
{
namespace Core
{
class UObject;
}
namespace Mobile
{
class Character;
}
namespace Network
{
class Client;

class PktSender
{
public:
  virtual ~PktSender() = default;
  virtual void Send( Client* client ) = 0;
};

class SendWorldItem final : public PktSender
{
public:
  SendWorldItem( u32 serial, u16 graphic, u16 amount, Core::Pos3d pos, u8 facing, u16 color,
                 u8 flags );
  virtual ~SendWorldItem() = default;
  virtual void Send( Client* client ) override;
  void updateFlags( u8 flags );

private:
  void buildF3();
  void build1A();
  u16 _p_oldlen;
  u32 _serial;
  u16 _graphic;
  u16 _amount;
  Core::Pos3d _pos;
  u8 _facing;
  u16 _color;
  u8 _flags;
  PktHelper::PacketOut<PktOut_1A> _p_old;
  PktHelper::PacketOut<PktOut_F3> _p;
};

class SendWorldMulti final : public PktSender
{
public:
  SendWorldMulti( u32 serial_ext, u16 graphic, Core::Pos3d pos, u16 color );
  virtual ~SendWorldMulti() = default;
  virtual void Send( Client* client ) override;

private:
  void buildF3();
  void build1A();
  u16 _p_oldlen;
  u32 _serial_ext;
  u16 _graphic;
  Core::Pos3d _pos;
  u16 _color;
  PktHelper::PacketOut<PktOut_1A> _p_old;
  PktHelper::PacketOut<PktOut_F3> _p;
};

class AddItemContainerMsg final : public PktSender
{
public:
  AddItemContainerMsg( u32 serial_ext, u16 graphic, u16 amount, Core::Pos2d pos, u8 slotindex,
                       u32 containerserial_ext, u16 color );
  virtual ~AddItemContainerMsg() = default;
  virtual void Send( Client* client ) override;

private:
  void buildLegacy();
  void build();
  u32 _serial_ext;
  u16 _graphic;
  u16 _amount;
  Core::Pos2d _pos;
  u8 _slotindex;
  u32 _containerserial_ext;
  u16 _color;
  PktHelper::PacketOut<PktOut_25> _p_old;
  PktHelper::PacketOut<PktOut_25> _p;
};

class MobileAnimationMsg final : public PktSender
{
public:
  MobileAnimationMsg( u32 serial_ext, u16 anim, u16 action, u8 subaction, u16 action_old,
                      u16 framecount_old, u16 repeat_old, Core::DIRECTION_FLAG_OLD backward_old,
                      Core::REPEAT_FLAG_OLD repeat_flag_old, u8 delay_old, bool oldanim_valid,
                      bool newanim_valid );
  MobileAnimationMsg( u32 serial_ext );
  virtual ~MobileAnimationMsg() = default;
  virtual void Send( Client* client ) override;
  void update( u16 anim, u16 action, u8 subaction, u16 action_old, u16 framecount_old,
               u16 repeat_old, Core::DIRECTION_FLAG_OLD backward_old,
               Core::REPEAT_FLAG_OLD repeat_flag_old, u8 delay_old, bool oldanim_valid,
               bool newanim_valid );

private:
  u32 _serial_ext;
  u16 _anim;
  u16 _action;
  u8 _subaction;
  u16 _action_old;
  u16 _framecount_old;
  u16 _repeat_old;
  u8 _backward_old;
  u8 _repeat_flag_old;
  u8 _delay_old;
  bool _oldanim_valid;
  bool _newanim_valid;
  void build();
  void build6E();

  PktHelper::PacketOut<PktOut_6E> _p_old;
  PktHelper::PacketOut<PktOut_E2> _p;
};

class PlaySoundPkt final : public PktSender
{
public:
  PlaySoundPkt( u8 type, u16 effect, Core::Pos3d center );
  virtual ~PlaySoundPkt() = default;
  virtual void Send( Client* client ) override;

private:
  void build();
  u8 _type;
  u16 _effect;
  Core::Pos3d _center;
  PktHelper::PacketOut<PktOut_54> _p;
};

class RemoveObjectPkt final : public PktSender
{
public:
  RemoveObjectPkt( u32 serial );
  virtual ~RemoveObjectPkt() = default;
  virtual void Send( Client* client ) override;
  void update( u32 serial );

private:
  void build();
  u32 _serial;
  PktHelper::PacketOut<PktOut_1D> _p;
};

class SendDamagePkt final : public PktSender
{
public:
  SendDamagePkt( u32 serial, u16 damage );
  virtual ~SendDamagePkt() = default;
  virtual void Send( Client* client ) override;

private:
  void build();
  void buildold();

  u32 _serial;
  u16 _damage;
  PktHelper::PacketOut<PktOut_BF_Sub22> _p_old;
  PktHelper::PacketOut<PktOut_0B> _p;
};

class ObjRevisionPkt final : public PktSender
{
public:
  ObjRevisionPkt( u32 serial_ext, u32 rev );
  virtual ~ObjRevisionPkt() = default;
  virtual void Send( Client* client ) override;

private:
  void build();
  void buildold();

  u32 _serial_ext;
  u32 _rev;
  PktHelper::PacketOut<PktOut_BF_Sub10> _p_old;
  PktHelper::PacketOut<PktOut_DC> _p;
};

class GraphicEffectPkt final : public PktSender
{
public:
  GraphicEffectPkt();
  virtual ~GraphicEffectPkt() = default;

  void movingEffect( const Core::UObject* src, const Core::UObject* dst, u16 effect, u8 speed,
                     u8 loop, u8 explode );
  void movingEffect( Core::Pos3d src, Core::Pos3d dst, u16 effect, u8 speed, u8 loop, u8 explode );
  void lightningBold( const Core::UObject* center );
  void followEffect( const Core::UObject* center, u16 effect, u8 speed, u8 loop );
  void stationaryEffect( Core::Pos3d src, u16 effect, u8 speed, u8 loop, u8 explode );

  virtual void Send( Client* client ) override;

private:
  void build();

  u8 _effect_type;
  u32 _src_serial_ext;
  u32 _dst_serial_ext;
  u16 _effect;
  Core::Pos3d _src;
  Core::Pos3d _dst;
  u8 _speed;
  u8 _loop;
  u8 _explode;
  u8 _unk26;

  PktHelper::PacketOut<PktOut_70> _p;
};

class GraphicEffectExPkt final : public PktSender
{
public:
  GraphicEffectExPkt();
  virtual ~GraphicEffectExPkt() = default;

  void movingEffect( const Core::UObject* src, const Core::UObject* dst, u16 effect, u8 speed,
                     u8 duration, u32 hue, u32 render, u8 direction, u8 explode, u16 effect3d,
                     u16 effect3dexplode, u16 effect3dsound );
  void movingEffect( Core::Pos3d src, Core::Pos3d dst, u16 effect, u8 speed, u8 duration, u32 hue,
                     u32 render, u8 direction, u8 explode, u16 effect3d, u16 effect3dexplode,
                     u16 effect3dsound );
  void followEffect( const Core::UObject* center, u16 effect, u8 speed, u8 duration, u32 hue,
                     u32 render, u8 layer, u16 effect3d );
  void stationaryEffect( Core::Pos3d pos, u16 effect, u8 speed, u8 duration, u32 hue, u32 render,
                         u16 effect3d );

  virtual void Send( Client* client ) override;

private:
  void build();

  u8 _effect_type;
  u32 _src_serial_ext;
  u32 _dst_serial_ext;
  u16 _effect;
  Core::Pos3d _src;
  Core::Pos3d _dst;
  u8 _speed;
  u8 _duration;
  u8 _direction;
  u8 _explode;
  u32 _hue;
  u32 _render;
  u16 _effect3d;
  u16 _effect3dexplode;
  u16 _effect3dsound;
  u32 _itemid;
  u8 _layer;

  PktHelper::PacketOut<PktOut_C7> _p;
};

class HealthBarStatusUpdate final : public PktSender
{
public:
  enum Color : u8
  {
    GREEN = 1,
    YELLOW = 2,
    RED = 3,
  };
  HealthBarStatusUpdate( u32 serial_ext, Color color, bool enable );
  virtual ~HealthBarStatusUpdate() = default;
  virtual void Send( Client* client ) override;

private:
  void build();

  u32 _serial_ext;
  bool _enable;
  Color _color;
  PktHelper::PacketOut<PktOut_17> _p;
};

class MoveChrPkt final : public PktSender
{
public:
  MoveChrPkt( const Mobile::Character* chr );
  virtual ~MoveChrPkt() = default;
  virtual void Send( Client* client ) override;

private:
  void build();

  const Mobile::Character* _chr;
  PktHelper::PacketOut<PktOut_77> _p;
};
}  // namespace Network
}  // namespace Pol
#endif
