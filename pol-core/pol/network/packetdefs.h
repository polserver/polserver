
#ifndef POL_PACKETDEFS_H
#define POL_PACKETDEFS_H

#include "packets.h"
#include "../../clib/rawtypes.h"

namespace Pol {
  namespace Mobile {
    class Character;
  }
  namespace Network {
    class Client;

    class PktSender
    {
    public:
      virtual void Send( Client* client ) =0;
    };

    class SendWorldItem : public PktSender
    {
    public:
      SendWorldItem( u32 serial, u16 graphic, u16 amount, u16 x, u16 y, s8 z, u8 facing, u16 color, u8 flags );
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
      enum DIRECTION_FLAG_OLD { BACKWARD = 1, FORWARD = 0 };
      enum REPEAT_FLAG_OLD { REPEAT = 1, NOREPEAT = 0 };

      MobileAnimationMsg( u32 serial_ext, u16 anim, u16 action, u8 subaction,
                          u16 action_old, u16 framecount_old, u16 repeat_old,
                          DIRECTION_FLAG_OLD backward_old, REPEAT_FLAG_OLD repeat_flag_old, u8 delay_old,
                          bool oldanim_valid, bool newanim_valid);
      MobileAnimationMsg( u32 serial_ext );
      virtual void Send( Client* client ) POL_OVERRIDE;
      void update( u16 anim, u16 action, u8 subaction,
                   u16 action_old, u16 framecount_old, u16 repeat_old,
                   DIRECTION_FLAG_OLD backward_old, REPEAT_FLAG_OLD repeat_flag_old, u8 delay_old,
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
	  virtual void Send( Client* client ) POL_OVERRIDE;
	  void update( u32 serial );
	private:
	  void build();
	  u32 _serial;
	  PktHelper::PacketOut<PktOut_1D> _p;
	};
  }
}
#endif
