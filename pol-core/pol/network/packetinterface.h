#ifndef __PACKETINTERFACE_H
#define __PACKETINTERFACE_H

#include "../../clib/rawtypes.h"
#include <map>
#include <queue>

namespace Pol
{
namespace Network
{
// interface for packets
class PacketInterface
{
public:
  PacketInterface() : offset( 0 ){};
  virtual ~PacketInterface() = default;
  u16 offset;
  virtual void ReSetBuffer(){};
  virtual char* getBuffer() { return nullptr; };
  virtual inline u8 getID() const { return 0; };
  virtual inline u16 getSize() const { return 0; };
  virtual inline u16 getSubID() const { return 0; };
  virtual size_t estimateSize() const { return 0; }
};

typedef std::queue<PacketInterface*> PacketInterfaceQueue;
typedef std::map<u16, PacketInterfaceQueue> PacketInterfaceQueueMap;
typedef std::pair<u16, PacketInterfaceQueue> PacketInterfaceQueuePair;

// interface for the two different types of packetqueues ("normal" packets
// and packets with subs)
class PacketQueue
{
public:
  PacketQueue() = default;
  virtual ~PacketQueue() = default;

public:
  virtual PacketInterface* GetNext( u8 id, u16 sub = 0 )
  {
    (void)id;
    (void)sub; /* unused variables */

    return nullptr;
  };
  virtual void Add( PacketInterface* pkt ) { (void)pkt; /*do nothing*/ };
  virtual size_t Count() const { return 0; };
  virtual bool HasSubs() const { return false; };
  virtual PacketInterfaceQueueMap* GetSubs() { return nullptr; };
  virtual size_t estimateSize() const = 0;
};


typedef std::pair<u8, PacketQueue*> PacketQueuePair;
typedef std::map<u8, PacketQueue*> PacketQueueMap;

// singleton "holder" of packets !EntryPoint!
class PacketsSingleton
{
public:
  PacketsSingleton();
  ~PacketsSingleton();

  PacketsSingleton( const PacketsSingleton& ) = delete;
  PacketsSingleton& operator=( const PacketsSingleton& ) = delete;

private:
  PacketQueueMap packets;

public:
  PacketInterface* getPacket( u8 id, u16 sub = 0 );
  void ReAddPacket( PacketInterface* pkt );
  PacketQueueMap* getPackets() { return &packets; };
  size_t estimateSize() const;
};
}  // namespace Network
}  // namespace Pol
#endif
