/*
History
=======
2007/04/07 MuadDib:   send_object_cache_to_inrange updated from just UObject* to
                      const UObject* for compatibility across more areas.

Notes
=======

*/

#ifndef __TOOLTIPS_H
#define __TOOLTIPS_H

namespace Pol {
  namespace Network {
    class Client;
  }
  namespace Core {
    class UObject;
	void send_object_cache( Network::Client* client, const UObject* obj );
	void send_object_cache_to_inrange( const UObject* obj );
    void SendAOSTooltip( Network::Client* client, UObject* item, bool vendor_content = false );
  }
}
#endif
