#ifndef UTIL_C
#define UTIL_C

#include "util.h"

extern Game game;

#define MAXBFSQUEUESIZE MAXMAPSIZE

static int  bfs_result[MAXBFSQUEUESIZE];
static Node bfs_queue[MAXBFSQUEUESIZE];


int _str_len(const char* str){
    if(str == NULL) return 0;
    int len = 0;
    for(; str[len]; len+=1);
    return len;
}

static int feed_uint(char* output, uint64_t u, int max_len){
    
    if(!output) return 0;
    
    int len = 0;

    uint64_t _10n = 1;
    for(; (u / 10) / _10n; _10n *= 10);

    for(; _10n && len < max_len; _10n /= 10){
        const int beforedigit = (u / 10) / _10n;
        const int afterdigit  = u - beforedigit * _10n * 10;
        const int digit = afterdigit / _10n ;
        output[len++] = digit + '0';
    }

    return len;
}

int _feed_str(char* output, int max_len, const char* input, const feed_str_arg_t args){
    int arg = 0;
    int len = 0;
    
    int i = 0;
    while(input[i] && len < max_len){
        for(; input[i] && len < max_len && input[i] != '%'; i+=1){
            output[len] = input[i];
            len += 1;
        }
        if(input[i] == '%'){
            i+=1;
            if(arg >= ARLEN(args.arg)){
                ERROR("formated string %s contains more formats %c than supported(%i)", input, '%', (int) ARLEN(args.arg));
                return len;
            }
            else switch (input[i])
            {
            case 's':
                if(args.arg[arg].str == NULL){
                    const char str_null[] = "(null)";
                    for(int i = 0; i < ARLEN(str_null) - 1 && len < max_len; i+=1)
                        output[len++] = str_null[i];
                }
                else for(int j = 0; args.arg[arg].str[j] && len < max_len; j+=1){
                    output[len] = args.arg[arg].str[j];
                    len += 1;
                }
                arg+=1;
                break;
            case 'i':
                if(args.arg[arg].i < 0){
                    output[len++] = '-';
                }
            case 'u':
                len += feed_uint(output + len, (input[i] == 'i')? ABS(args.arg[arg].i) : args.arg[arg].u, max_len);
                printf("%i\n", args.arg[arg].i);
                break;
            default:
                ERROR("format '%c' not implemented\n", input[i]);
                break;
            }
            i+=1;
        }
    }
    if(len >= max_len) REPORT(WARN, "in _feed_str strlen(%i) exceded max(%i)", len, max_len);
    return len;
}

int in_bounds(const Rect rect, int x, int y){
    return (x > rect.x && x < rect.x + rect.w) && (y > rect.y && y < rect.y + rect.h);
}

int in_sbounds(const Surface surface, int x, int y){
    return (x > -1 && x < surface.w) && (y > -1 && y < surface.h);
}

int in_mbounds(const Map map, int x, int y){
    return (x > -1 && x < map.w) && (y > -1 && y < map.h);
}

int collide_rect(const Rect rect1, const Rect rect2){
    if(in_bounds(rect1, rect2.x, rect2.y)) return 1;
    if(in_bounds(rect1, rect2.x + rect2.w, rect2.y)) return 1;
    if(in_bounds(rect1, rect2.x, rect2.y + rect2.h)) return 1;
    if(in_bounds(rect1, rect2.x + rect2.w, rect2.y + rect2.h)) return 1;
    return 0;
}

int distance2(int x1, int y1, int x2, int y2){
    return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
}

float Q_rsqrt(float number) {
    const float x2 = number * 0.5f;
    const float threehalfs = 1.5f;

    const int i = 0x5f3759df - ((*(int*) &number) >> 1);

    float f = *(float*) &i;

    f = f * (threehalfs - (x2 * f * f));
    f = f * (threehalfs - (x2 * f * f));
    f = f * (threehalfs - (x2 * f * f));
    f = f * (threehalfs - (x2 * f * f));
    f = f * (threehalfs - (x2 * f * f));

    return f;
}

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


int find_path(int* output, int range, const Map map, int originx, int originy, int targetx, int targety){


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

#endif // =====================  END OF FILE UTIL_C ===========================