/** @file
 *
 * @par History
 */


#ifndef WTHRTYPE_H
#define WTHRTYPE_H

namespace Pol::Core
{
class WeatherRegion;

void SetRegionWeatherLevel( WeatherRegion* weatherregion, unsigned type, unsigned severity,
                            unsigned aux, int lightoverride );
}  // namespace Pol::Core

#endif
