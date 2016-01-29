/** @file
 *
 * @par History
 */


#ifndef WTHRTYPE_H
#define WTHRTYPE_H
namespace Pol {
  namespace Core {
	class WeatherRegion;

	void SetRegionWeatherLevel( WeatherRegion* weatherregion,
								unsigned type,
								unsigned severity,
								unsigned aux,
								int lightoverride );
  }
}
#endif
