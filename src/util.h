#ifndef UTIL_HEADER
#define UTIL_HEADER

#include "begin.h"
#include "maps.h"

int _str_len(const char* str);

int feed_str(char* output, const char* input, int max_len);

int in_bounds(const Rect rect, int x, int y);

int in_sbounds(const Surface surface, int x, int y);

int in_mbounds(const Map map, int x, int y);

int collide_rect(const Rect rect1, const Rect rect2);

int distance2(int x1, int y1, int x2, int y2);

float Q_rsqrt(float number);

// performs bfs from (x, y) storing the result in bfs_result
// \returns the index of the last node in the bfs_queue
int bfs_from(const Map map, int x, int y, int range);

// puts the path from (originx, originy) to (targetx, targety) in output
// \returns thse size of the path or -1 if no path is found
int find_path(int* output, int range, const Map map, int originx, int originy, int targetx, int targety);

Ray prepare_ray(int x, int y, int targetx, int targety);

// performs a ray_cast step in the horizontal direction
// \param ray a pointer to the ray, use prepare_ray to create such ray
// \param map the map in which the ray is being casted
// \returns wether the ray has hit a tile with visible type
int ray_steph(const Map map, Ray* ray);

// performs a ray_cast step in the vertical direction
// \param ray a pointer to the ray, use prepare_ray to create such ray
// \param map the map in which the ray is being casted
// \returns wether the ray has hit a tile with visible type
int ray_stepv(const Map map, Ray* ray);

#endif // =====================  END OF FILE UTIL_HEADER ===========================