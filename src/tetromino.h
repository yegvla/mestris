#ifndef TETROMINO_H
#define TETROMINO_H

#include "gfx.h"
#include <gpu.h>
#include <stdint.h>

typedef enum {
    I = 0,
    J = 1,
    L = 2,
    O = 3,
    S = 4,
    T = 5,
    Z = 6,
    NONE = 7,
} shape;

typedef enum { AIR = 0, FILLED = 1, BORDER = 2, GHOST = 3, GARBAGE = 4 } texture;

typedef enum { DEG_0 = 0, DEG_90 = 1, DEG_180 = 2, DEG_270 = 3 } rotation;
typedef enum {
    COLOR_A = 0,
    COLOR_B = 1,
    COLOR_A_LIGHT = 2,
    COLOR_B_LIGHT = 3,
} color;

typedef struct {
    int8_t x;
    int8_t y;
} vec2i8;

/* 0,0 is the center of every piece.  Every specified coordinate will
 * be filled with one tile.  4 Coordinates make up 1 teromino.
 *
 *	-2,2	-1,2	0,2	1,2	2,2
 *	-2,1	-1,1	0,1	1,1	2,1
 *	-2,0	-1,0	0,0	1,0	2,0
 *	-2,-1	-1,-1	0,1	1,2	2,1
 *	-2,-2	-1,-2	0,2	1,2	2,2
 *
 * E.g. the I tetromino will be spawned like this:
 * {{-1, 0}, {0, 0}, {1, 0}, {2, 0}}
 *
 *	-2,2	-1,2	0,2	1,2	2,2
 *	-2,1	-1,1	0,1	1,1	2,1
 *	-2,0	[-1,0]	[0,0]	[1,0]	[2,0]
 *	-2,-1	-1,-1	0,1	1,2	2,1
 *	-2,-2	-1,-2	0,2	1,2	2,2
 *
 * The rotation matrix was copied from this source:
 * https://harddrop.com/wiki/SRS
 */

static const vec2i8 SHAPES[7][4][4] = {
    // I
    {
        {{-1, 0}, {0, 0}, {1, 0}, {2, 0}},  // 0°
        {{0, 1}, {0, 0}, {0, -1}, {0, -2}}, // 90°
        {{-2, 0}, {-1, 0}, {0, 0}, {1, 0}}, // 180°
        {{0, 2}, {0, 1}, {0, 0}, {0, -1}},  // 270°
    },
    // J
    {
        {{-1, 1}, {-1, 0}, {0, 0}, {1, 0}},  // 0°
        {{1, 1}, {0, 1}, {0, 0}, {0, -1}},   // 90°
        {{-1, 0}, {0, 0}, {1, 0}, {1, -1}},  // 180°
        {{-1, -1}, {0, -1}, {0, 0}, {0, 1}}, // 270°
    },
    // L
    {
        {{-1, 0}, {0, 0}, {1, 0}, {1, 1}},   // 0°
        {{0, 1}, {0, 0}, {0, -1}, {1, -1}},  // 90°
        {{-1, -1}, {-1, 0}, {0, 0}, {1, 0}}, // 180°
        {{-1, 1}, {0, 1}, {0, 0}, {0, -1}},  // 270°
    },
    // O
    {
        {{0, 1}, {1, 1}, {0, 0}, {1, 0}},     // 0°
        {{0, 0}, {1, 0}, {0, -1}, {1, -1}},   // 90°
        {{-1, 0}, {0, 0}, {-1, -1}, {0, -1}}, // 180°
        {{-1, 1}, {0, 1}, {-1, 0}, {0, 0}},   // 270°
    },
    // S
    {
        {{-1, 0}, {0, 0}, {0, 1}, {1, 1}},   // 0°
        {{0, 1}, {0, 0}, {1, 0}, {1, -1}},   // 90°
        {{-1, -1}, {0, -1}, {0, 0}, {1, 0}}, // 180°
        {{-1, 1}, {-1, 0}, {0, 0}, {0, -1}}, // 270°
    },
    // T
    {
        {{-1, 0}, {0, 0}, {0, 1}, {1, 0}},  // 0°
        {{0, 1}, {0, 0}, {0, -1}, {1, 0}},  // 90°
        {{-1, 0}, {0, 0}, {0, -1}, {1, 0}}, // 180°
        {{0, 1}, {0, 0}, {-1, 0}, {0, -1}}, // 270°
    },
    // Z
    {
        {{-1, 1}, {0, 1}, {0, 0}, {1, 0}},   // 0°
        {{1, 1}, {1, 0}, {0, 0}, {0, -1}},   // 90°
        {{-1, 0}, {0, 0}, {0, -1}, {1, -1}}, // 180°
        {{0, 1}, {0, 0}, {-1, 0}, {-1, -1}}, // 270°
    }};

static const vec2i8 OFFSET_DATA_JLSTZ[4][5] = {
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
    {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
};

static const vec2i8 OFFSET_DATA_I[4][5] = {
    {{0, 0}, {-1, 0}, {2, 0}, {-1, 0}, {2, 0}},
    {{-1, 0}, {0, 0}, {0, 0}, {0, 1}, {0, -2}},
    {{-1, 1}, {1, 1}, {-2, 1}, {1, 0}, {-2, 0}},
    {{0, 1}, {0, 1}, {0, 1}, {0, -1}, {0, 2}},
};

static const vec2i8 OFFSET_DATA_O[4] = {
    {0, 0},
    {0, -1},
    {-1, -1},
    {-1, 0},
};

typedef struct {
    texture texture;
    color color;
} tile;

typedef struct {
    shape shape;
    rotation rotation;
    tile style;
} tetromino;

tetromino tetromino_create(shape shape, texture texture, color color);

tetromino tetromino_create_empty(void);

tetromino tetromino_random(void);

void tetromino_random_bag(tetromino *bag);

const pixel *texture_get_pixles(texture texture);

color pixel_apply_color(pixel pixel, color color);

color color_get_light_var(color color);

color color_get_normal_var(color color);

#endif // TETROMINO_H
