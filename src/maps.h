#ifndef MAPS_HEADER
#define MAPS_HEADER

#include "begin.h"

#define MAXBFSQUEUESIZE (sizeof(map1) / sizeof(map1[0]))

#define MAXMAPSIZE (sizeof(map1) / sizeof(map1[0]))

static const int map1w = 32;
static const int map1h = 32;
static const unsigned char map1[] = {
	01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01,
	01, 16, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01,
	01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01,
	01, 00, 00, 01, 01, 01, 01, 01, 00, 01, 01, 01, 01, 01, 00, 00, 00, 00, 01, 01, 01, 01, 01, 00, 01, 01, 01, 01, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 17, 00, 00, 00, 00, 00, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 01, 01, 01, 01, 00, 01, 01, 01, 01, 01, 00, 00, 00, 00, 01, 01, 01, 01, 01, 00, 01, 01, 01, 01, 01, 00, 00, 01,
	01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01,
	01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01,
	01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 17, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01,
	01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01,
	01, 00, 00, 01, 01, 01, 01, 01, 00, 01, 01, 01, 01, 01, 00, 00, 00, 00, 01, 01, 01, 01, 01, 00, 01, 01, 01, 01, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 17, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 01,
	01, 00, 00, 01, 01, 01, 01, 01, 00, 01, 01, 01, 01, 01, 00, 00, 00, 00, 01, 01, 01, 01, 01, 00, 01, 01, 01, 01, 01, 00, 00, 01,
	01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01,
	01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01,
	01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01,
};

static int  bfs_result[MAXBFSQUEUESIZE];
static Node bfs_queue[MAXBFSQUEUESIZE];
static uint32_t mapbuff[MAXMAPSIZE];


// performs bfs from (x, y) storing the result in bfs_result
// \param callback 
// \returns the index of the last node in the bfs_queue
int bfs_from(const Map map, int x, int y, int range, void(*callback)(int node)){
	
    const Node origin = (Node){.parent = -1, .x = x, .y = y};

    const int j0     = MAX(0, x);
    const int xrange = MIN(x + range, map.w);
    const int yrange = MIN(y + range, map.h);

    // init distances
    for (int i = MAX(0, y); i < yrange; i+=1)
        for (int j = j0; j < xrange; j+=1)
            bfs_result[i * map.w + j] = -1;
    
    int qh=0, qt=0;

    bfs_queue[qt++] = origin;
    bfs_result[origin.y * map.w + origin.x] = 0;

    int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

    Node cur = origin;

    while (qh < qt && bfs_result[cur.y * map.w + cur.x] < range) {
        
        for (int i=0; i<4; i++) {
            int nx = cur.x + dirs[i][0];
            int ny = cur.y + dirs[i][1];
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

    const int j0     = MAX(0, originx);
    const int xrange = MIN(originx + range, map.w);
    const int yrange = MIN(originy + range, map.h);

    // init distances
    for (int i = MAX(0, originy); i < yrange; i+=1)
        for (int j = j0; j < xrange; j+=1)
            bfs_result[i * map.w + j] = -1;
    
    int qh=0, qt=0;

    bfs_queue[qt++] = origin;
    bfs_result[origin.y * map.w + origin.x] = 0;

    int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

    Node cur = origin;

    while (qh < qt && bfs_result[cur.y * map.w + cur.x] < range) {
        
        for (int i=0; i<4; i++) {
            int nx = cur.x + dirs[i][0];
            int ny = cur.y + dirs[i][1];
            if(nx == targetx && ny == targety){
                int path_size = 0;
                while(cur.parent >= 0){
                    output[path_size++] = cur.y * map.w + cur.x;
                    cur = bfs_queue[cur.parent];
                }
                output[path_size++] = targety * map.w + targetx;
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

#endif // =====================  END OF FILE MAPS_HEADER ===========================