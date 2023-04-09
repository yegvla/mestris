#ifndef TETROMINO_H
#define TETROMINO_H

#include "gfx.h"
#include <gpu.h>
#include <stdint.h>

typedef enum { I = 0, J = 1, L = 2, O = 3, S = 4, T = 5, Z = 6 } shape_t;
typedef enum {
    AIR = 0,
    FILLED = 1,
    BORDER = 2,
    GHOST = 3,
    GARBAGE = 4
} texture_t;
typedef enum { DEG_0 = 0, DEG_90 = 1, DEG_180 = 2, DEG_270 = 3 } rotation_t;
typedef enum { COLOR_A = 0, COLOR_B = 1 } color_t;

typedef struct {
    int8_t x;
    int8_t y;
} vec2i8;

#define VEC2_ADD(LHS, RHS)                                                     \
    (typeof(LHS)) { .x = (LHS).x + (RHS).x, .y = (LHS).y + (RHS).y, }

#define VEC2_SUB(LHS, RHS)                                                     \
    (typeof(LHS)) { .x = (LHS).x - (RHS).x, .y = (LHS).y - (RHS).y, }


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
    texture_t texture;
    color_t color;
} tile_t;

typedef struct {
    shape_t shape;
    rotation_t rotation;
    tile_t style;
} tetromino_t;

tetromino_t tetromino_create(shape_t shape, texture_t texture, color_t color);

const pixel_t* texture_get_pixles(texture_t texture);

color_t pixel_apply_color(pixel_t pixel, color_t color);

#endif // TETROMINO_H
