/*
 * wcx_common.h
 *
 */

#ifndef EXT_WCX_WCX_COMMON_H_
#define EXT_WCX_WCX_COMMON_H_

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define D_R (M_PI / 180.0)

double geoDistance(double lat1d, double lon1d, double lat2d, double lon2d);

#endif /* EXT_WCX_WCX_COMMON_H_ */
