/*
History
=======


Notes
=======

*/

#ifndef WTHRTYPE_H
#define WTHRTYPE_H

class WeatherRegion;

void SetRegionWeatherLevel( WeatherRegion* weatherregion,
                            unsigned type,
                            unsigned severity,
                            unsigned aux,
                            int lightoverride );

#endif
