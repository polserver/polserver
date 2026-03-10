#ifndef __PACKETHELPER_H
#define __PACKETHELPER_H

#include "../globals/network.h"
#include "client.h"
#include "clienttransmit.h"
#include "packets.h"


namespace Pol::Network::PktHelper
{
using namespace PacketWriterDefs;
// creates new packets
PacketInterface* GetPacket( u8 id, u16 sub = 0 );

template <class T>
inline T* RequestPacket( u8 id, u16 sub = 0 )
{
  return static_cast<T*>( Core::networkManager.packetsSingleton->getPacket( id, sub ) );
};

inline void ReAddPacket( PacketInterface* msg )
{
  Core::networkManager.packetsSingleton->ReAddPacket( msg );
};

template <class T>
class PacketOut
{
private:
  T* pkt;

public:
  PacketOut();
  ~PacketOut();
  PacketOut( const PacketOut<T>& ) = delete;
  PacketOut<T>& operator=( const PacketOut<T>& ) = delete;

  void Release();
  void Send( Client* client, int len = -1 ) const;
  T* operator->() const;
  T* Get();
};

template <class T>
PacketOut<T>::PacketOut() : pkt( RequestPacket<T>( T::ID, T::SUB ) )
{
}

template <class T>
PacketOut<T>::~PacketOut()
{
  if ( pkt )
    ReAddPacket( pkt );
}

template <class T>
void PacketOut<T>::Release()
{
  ReAddPacket( pkt );
  pkt = nullptr;
}

template <class T>
void PacketOut<T>::Send( Client* client, int len ) const
{
  if ( !pkt )
    return;
  if ( len == -1 )
    len = pkt->offset;
  Core::networkManager.clientTransmit->AddToQueue( client, &pkt->buffer, len );
}

template <class T>
T* PacketOut<T>::operator->() const
{
  return pkt;
}

template <class T>
T* PacketOut<T>::Get()
{
  return pkt;
}
}  // namespace Pol::Network::PktHelper


#endif
