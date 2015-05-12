/*
 * wcx_common.c
 *
 */

#include <math.h>
#include "wcx_common.h"

const double EARTH_RADIUS_IN_METERS = 6372797.560856;

static inline double deg_rad(double ang) {
    return ang * D_R;
}

double geoDistance(double lat1d, double lon1d, double lat2d, double lon2d) {
    double lat1r, lon1r, lat2r, lon2r, u, v;
    lat1r = deg_rad(lat1d);
    lon1r = deg_rad(lon1d);
    lat2r = deg_rad(lat2d);
    lon2r = deg_rad(lon2d);
    u = sin((lat2r - lat1r) / 2);
    v = sin((lon2r - lon1r) / 2);

    return 2.0 * EARTH_RADIUS_IN_METERS * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}
