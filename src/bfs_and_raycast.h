#ifndef RAYCAST_HEADER
#define RAYCAST_HEADER

#include "game.h"
#include "maps.h"

#define MAXBFSQUEUESIZE (sizeof(map1) / sizeof(map1[0]))

static int  bfs_result[MAXBFSQUEUESIZE];
static Node bfs_queue[MAXBFSQUEUESIZE];


// performs bfs from (x, y) storing the result in bfs_result
// \returns the index of the last node in the bfs_queue
int bfs_from(const Map map, int x, int y, int range){
	
    const Node origin = (Node){.parent = -1, .x = x, .y = y};

    const int j0     = MAX(0, x - range);
    const int xrange = MIN(x + range, map.w);
    const int yrange = MIN(y + range, map.h);

    // init distances
    for (int i = MAX(0, y - range); i < yrange; i+=1)
        for (int j = j0; j < xrange; j+=1)
            bfs_result[i * map.w + j] = -1;
    
    int qh=0, qt=0;

    bfs_queue[qt++] = origin;
    bfs_result[origin.y * map.w + origin.x] = 0;

    const int dirs[8] = {
         1,  0,
        -1,  0,
         0,  1,
         0, -1
    };

    Node cur = origin;
    
    while (qh < qt && bfs_result[cur.y * map.w + cur.x] < range) {
        
        for (int i=0; i<4; i++) {
            const int nx = cur.x + dirs[2 * i + 0];
            const int ny = cur.y + dirs[2 * i + 1];
            if (nx>=0 && nx<map.w && ny>=0 && ny<map.h &&
                map.map[ny * map.w + nx]==0 && bfs_result[ny * map.w + nx]==-1) {
                bfs_result[ny * map.w + nx] = bfs_result[cur.y * map.w + cur.x] + 1;
                bfs_queue[qt++] = (Node){.parent = qh, .x = nx, .y = ny};
            }
        }
        cur = bfs_queue[++qh];
    }
    return qh;
}

// puts the path from (originx, originy) to (targetx, targety) in output
// \returns thse size of the path or -1 if no path is found
int find_path(int* output, int range, const Map map, int originx, int originy, int targetx, int targety){

    //printf("find path from (%i, %i) -> (%i, %i)\n", originx, originy, targetx, targety);

    if(originx == targetx && originy == targety) return 0;

    const Node origin = (Node){.parent = -1, .x = originx, .y = originy};

    const int j0     = MAX(0, originx - range);
    const int xrange = MIN(originx + range, map.w);
    const int yrange = MIN(originy + range, map.h);

    // init distances
    for (int i = MAX(0, originy - range); i < yrange; i+=1)
        for (int j = j0; j < xrange; j+=1)
            bfs_result[i * map.w + j] = -1;
    
    int qh=0, qt=0;

    bfs_queue[qt++] = origin;
    bfs_result[origin.y * map.w + origin.x] = 0;

    const int dirs[8] = {
         1,  0,
        -1,  0,
         0,  1,
         0, -1
    };

    Node cur = origin;

    while (qh < qt && bfs_result[cur.y * map.w + cur.x] < range) {
        
        for (int i=0; i<4; i++) {
            const int nx = cur.x + dirs[2 * i + 0];
            const int ny = cur.y + dirs[2 * i + 1];
            if(nx == targetx && ny == targety){
                int path_size = 0;
                output[path_size++] = targety * map.w + targetx;
                while(cur.parent >= 0){
                    output[path_size++] = cur.y * map.w + cur.x;
                    cur = bfs_queue[cur.parent];
                }
                return path_size;
            }
            if (nx>=0 && nx<map.w && ny>=0 && ny<map.h &&
                map.map[ny * map.w + nx]==0 && bfs_result[ny * map.w + nx]==-1) {
                bfs_result[ny * map.w + nx] = bfs_result[cur.y * map.w + cur.x] + 1;
                bfs_queue[qt++] = (Node){.parent = qh, .x = nx, .y = ny};
            }
        }
        cur = bfs_queue[++qh];
    }
    //printf("no path found\n");
    return -1;
}

Ray prepare_ray(int x, int y, int targetx, int targety){
    return (Ray){
        .x = (float) x,
        .y = (float) y,
        .r = 0,
        .cos = (float)(targetx - x) / sqrtf((float) ((targetx - x) * (targetx - x) + (targety - y) * ( targety - y))),
        .sin = (float)(targety - y) / sqrtf((float) ((targetx - x) * (targetx - x) + (targety - y) * ( targety - y)))
    };
}

// performs a ray_cast step in the horizontal direction
// \param ray a pointer to the ray, use prepare_ray to create such ray
// \param map the map in which the ray is being casted
// \returns wether the ray has hit a tile with visible type
int ray_steph(const Map map, Ray* ray){

    if(ray->cos == 0){
        if(ray->sin == 0) return -1;
        ray->y  = INFINITY;
        ray->r = INFINITY;
        return 0;
    }

    const float dx = (ray->cos > 0)? 1.0f * TILEW : -1.0f * TILEW;
    const float dy = (dx * ray->sin) / ray->cos;

    ray->x += dx;
    ray->y += dy;
    ray->r += sqrtf(dx * dx + dy * dy);

    return get_tile(map, (int) (ray->x / TILEW) , (int) (ray->y / TILEH));;
}

// performs a ray_cast step in the vertical direction
// \param ray a pointer to the ray, use prepare_ray to create such ray
// \param map the map in which the ray is being casted
// \returns wether the ray has hit a tile with visible type
int ray_stepv(const Map map, Ray* ray){

    if(ray->sin == 0){
        if(ray->cos == 0) return -1;
        ray->x  = INFINITY;
        ray->r = INFINITY;
        return 0;
    }

    const float dy = (ray->sin > 0)? 1.0f * TILEH : -1.0f * TILEH;
    const float dx = (dy * ray->cos) / ray->sin;

    ray->x += dx;
    ray->y += dy;
    ray->r += sqrtf(dx * dx + dy * dy);

    return get_tile(map, (int) (ray->x / TILEW) , (int) (ray->y / TILEH));
}


#endif // =====================  END OF FILE RAYCAST_HEADER ===========================
