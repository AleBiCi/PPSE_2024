#ifndef __SUN_AZ_ALT_H__
#define __SUN_AZ_ALT_H__

#include<time.h>
#include<math.h>
#ifndef M_PI
#define M_PI (3.14159265358979323846264338327950288)
#endif /* M_PI */

/*
Function that return current julian date (decimal)
*/
double julian_day(time_t utc_time_point);

/*
Function that calculate sun Azimuth and Altitude given a place Latitude, Longitude, Altitude
and time.
The result is pointed by "Az" and "El"
*/
void SolarAzEl(tm* time_info_ptr, double Lat, double Lon, double Alt, double* Az, double* El);

#endif

