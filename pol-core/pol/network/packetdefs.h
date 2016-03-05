
#ifndef POL_PACKETDEFS_H
#define POL_PACKETDEFS_H

#include "packets.h"
#include "packethelper.h"
#include "../../clib/rawtypes.h"
#include "../action.h"

namespace Pol {
  namespace Core {
    class UObject;
  }
  namespace Mobile {
    class Character;
  }
  namespace Network {
    class Client;

    class PktSender
    {
    public:
      virtual ~PktSender(){};
      virtual void Send( Client* client ) =0;
    };

    class SendWorldItem : public PktSender
    {
    public:
      SendWorldItem( u32 serial, u16 graphic, u16 amount, u16 x, u16 y, s8 z, u8 facing, u16 color, u8 flags );
      virtual ~SendWorldItem(){};
      virtual void Send( Client* client ) POL_OVERRIDE;
      void updateFlags( u8 flags );
    private:
      void buildF3();
      void build1A();
      u16 _p_oldlen;
      u32 _serial;
      u16 _graphic;
      u16 _amount;
      u16 _x;
      u16 _y;
      s8 _z;
      u8 _facing;
      u16 _color;
      u8 _flags;
      PktHelper::PacketOut<PktOut_1A> _p_old;
      PktHelper::PacketOut<PktOut_F3> _p;
    };

    class SendWorldMulti : public PktSender
    {
    public:
      SendWorldMulti( u32 serial_ext, u16 graphic, u16 x, u16 y, s8 z, u16 color );
      virtual ~SendWorldMulti(){};
      virtual void Send( Client* client ) POL_OVERRIDE;
    private:
      void buildF3();
      void build1A();
      u16 _p_oldlen;
      u32 _serial_ext;
      u16 _graphic;
      u16 _x;
      u16 _y;
      s8 _z;
      u16 _color;
      PktHelper::PacketOut<PktOut_1A> _p_old;
      PktHelper::PacketOut<PktOut_F3> _p;
    };

    class AddItemContainerMsg : public PktSender
    {
    public:
      AddItemContainerMsg( u32 serial_ext, u16 graphic, u16 amount, u16 x, u16 y, u8 slotindex, u32 containerserial_ext, u16 color );
      virtual ~AddItemContainerMsg(){};
      virtual void Send( Client* client ) POL_OVERRIDE;
    private:
      void buildLegacy();
      void build();
      u32 _serial_ext;
      u16 _graphic;
      u16 _amount;
      u16 _x;
      u16 _y;
      u8 _slotindex;
      u32 _containerserial_ext;
      u16 _color;
      PktHelper::PacketOut<PktOut_25> _p_old;
      PktHelper::PacketOut<PktOut_25> _p;
    };

    class MobileAnimationMsg : public PktSender
    {
    public:
      MobileAnimationMsg( u32 serial_ext, u16 anim, u16 action, u8 subaction,
                          u16 action_old, u16 framecount_old, u16 repeat_old,
                          Core::DIRECTION_FLAG_OLD backward_old, Core::REPEAT_FLAG_OLD repeat_flag_old, u8 delay_old,
                          bool oldanim_valid, bool newanim_valid);
      MobileAnimationMsg( u32 serial_ext );
      virtual ~MobileAnimationMsg(){};
      virtual void Send( Client* client ) POL_OVERRIDE;
      void update( u16 anim, u16 action, u8 subaction,
                   u16 action_old, u16 framecount_old, u16 repeat_old,
                   Core::DIRECTION_FLAG_OLD backward_old, Core::REPEAT_FLAG_OLD repeat_flag_old, u8 delay_old,
                   bool oldanim_valid, bool newanim_valid );
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

	class PlaySoundPkt : public PktSender
	{
	public:
	  PlaySoundPkt( u8 type, u16 effect, u16 xcenter, u16 ycenter, s16 zcenter );
          virtual ~PlaySoundPkt(){};
	  virtual void Send( Client* client ) POL_OVERRIDE;
	private:
	  void build();
	  u8 _type;
	  u16 _effect;
	  u16 _xcenter;
	  u16 _ycenter;
	  s16 _zcenter;
	  PktHelper::PacketOut<PktOut_54> _p;
	};

	class RemoveObjectPkt : public PktSender
	{
	public:
	  RemoveObjectPkt( u32 serial );
          virtual ~RemoveObjectPkt(){};
	  virtual void Send( Client* client ) POL_OVERRIDE;
	  void update( u32 serial );
	private:
	  void build();
	  u32 _serial;
	  PktHelper::PacketOut<PktOut_1D> _p;
	};

    class SendDamagePkt : public PktSender
    {
    public:
      SendDamagePkt(u32 serial, u16 damage);
      virtual ~SendDamagePkt(){};
      virtual void Send( Client* client ) POL_OVERRIDE;
    private:
      void build();
      void buildold();

      u32 _serial;
      u16 _damage;
      PktHelper::PacketOut<PktOut_BF_Sub22> _p_old;
      PktHelper::PacketOut<PktOut_0B> _p;
    };

    class ObjRevisionPkt : public PktSender
    {
      public:
        ObjRevisionPkt(u32 serial_ext, u32 rev);
        virtual ~ObjRevisionPkt(){};
        virtual void Send( Client* client ) POL_OVERRIDE;
      private:
        void build();
        void buildold();

        u32 _serial_ext;
        u32 _rev;
        PktHelper::PacketOut<PktOut_BF_Sub10> _p_old;
        PktHelper::PacketOut<PktOut_DC> _p;
    };

    class GraphicEffectPkt : public PktSender
    {
    public:
      GraphicEffectPkt();
      GraphicEffectPkt(u8 effect_type, u32 src_serial_ext, u32 dst_serial_ext,
        u16 effect, u16 xs, u16 ys, s8 zs, u16 xd, u16 yd, s8 zd,
        u8 speed, u8 loop, u8 explode, u8 unk26);
      virtual ~GraphicEffectPkt(){};

      void movingEffect(const Core::UObject* src, const Core::UObject* dst,
                             u16 effect, u8 speed, u8 loop, u8 explode);
      void movingEffect(u16 xs, u16 ys, s8 zs, u16 xd, u16 yd, s8 zd,
                             u16 effect, u8 speed, u8 loop, u8 explode);
      void lightningBold(const Core::UObject* center);
      void followEffect(const Core::UObject* center, u16 effect, u8 speed, u8 loop);
      void stationaryEffect(u16 xs, u16 ys, s8 zs, u16 effect, u8 speed, u8 loop, u8 explode);

      virtual void Send( Client* client ) POL_OVERRIDE;
    private:
       void build();

       u8 _effect_type;
       u32 _src_serial_ext;
       u32 _dst_serial_ext;
       u16 _effect;
       u16 _xs;
       u16 _ys;
       s8 _zs;
       u16 _xd;
       u16 _yd;
       s8 _zd;
       u8 _speed;
       u8 _loop;
       u8 _explode;
       u8 _unk26;

       PktHelper::PacketOut<PktOut_70> _p;       
    };

    class GraphicEffectExPkt : public PktSender
    {
    public:
      GraphicEffectExPkt();
      GraphicEffectExPkt(u8 effect_type, u32 src_serial_ext, u32 dst_serial_ext,
        u16 srcx, u16 srcy, s8 srcz,
		u16 dstx, u16 dsty, s8 dstz,
		u16 effect, u8 speed, u8 duration, u8 direction,
		u8 explode, u32 hue, u32 render,
		u16 effect3d, u16 effect3dexplode, u16 effect3dsound,
		u32 itemid, u8 layer);
      virtual ~GraphicEffectExPkt(){};

      void movingEffect(const Core::UObject *src, const Core::UObject *dst,
						u16 effect, u8 speed, u8 duration, u32 hue,
						u32 render, u8 direction, u8 explode,
						u16 effect3d, u16 effect3dexplode, u16 effect3dsound);
      void movingEffect(u16 xs, u16 ys, s8 zs,
                        u16 xd, u16 yd, s8 zd,
						u16 effect, u8 speed, u8 duration, u32 hue,
						u32 render, u8 direction, u8 explode,
						u16 effect3d, u16 effect3dexplode, u16 effect3dsound);
      void followEffect(const Core::UObject* center, u16 effect, u8 speed, u8 duration, u32 hue,
						u32 render, u8 layer, u16 effect3d);
      void stationaryEffect(u16 x, u16 y, s8 z, u16 effect, u8 speed, u8 duration, u32 hue,
							u32 render, u16 effect3d);

      virtual void Send( Client* client ) POL_OVERRIDE;
    private:
       void build();

       u8 _effect_type;
       u32 _src_serial_ext;
       u32 _dst_serial_ext;
       u16 _effect;
       u16 _xs;
       u16 _ys;
       s8 _zs;
       u16 _xd;
       u16 _yd;
       s8 _zd;
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

    class HealthBarStatusUpdate : public PktSender
    {
    public:
      enum Color : u8
      {
        GREEN  = 1,
        YELLOW = 2,
        RED    = 3,
      };
      HealthBarStatusUpdate(u32 serial_ext, Color color, bool enable);
      virtual ~HealthBarStatusUpdate(){};
      virtual void Send( Client* client ) POL_OVERRIDE;
    private:
      void build();

      u32 _serial_ext;
      bool _enable;
      Color _color;
      PktHelper::PacketOut<PktOut_17> _p;
    };

    class MoveChrPkt : public PktSender
    {
    public:
      MoveChrPkt(const Mobile::Character* chr);
      virtual ~MoveChrPkt(){};
      virtual void Send( Client* client ) POL_OVERRIDE;
    private:
      void build();

      const Mobile::Character* _chr;
      PktHelper::PacketOut<PktOut_77> _p;
    };
  }
}
#endif
