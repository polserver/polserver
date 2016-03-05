/** @file
 *
 * @par History
 */


#ifndef UOBJHELP_H
#define UOBJHELP_H
namespace Pol {
  namespace Network {
    class Client;
  }
  namespace Core {
	class UObject;
	class PropertyList;

	class UObjectHelper
	{
	public:
      static void ShowProperties( Network::Client* client, PropertyList& proplist );
      static void ShowProperties( Network::Client* client, UObject* obj );
	};
  }
}
#endif
