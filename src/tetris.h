#ifndef TETRIS_H
#define TETRIS_H

#include <stdint.h>

// Background and foreground colors, will always stay the same.
#define COLOR_BG 0
#define COLOR_FG 1

// Color A, will change every level
#define COLOR_A1 2
#define COLOR_A2 3
#define COLOR_A3 4

// Color B, will change every level
#define COLOR_B1 5
#define COLOR_B2 6
#define COLOR_B3 7

#define MAX(a, b)                                                              \
    ({                                                                         \
        __typeof__(a) _a = (a);                                                \
        __typeof__(b) _b = (b);                                                \
        _a > _b ? _a : _b;                                                     \
    })

#define LOCK_TIME (1000)
#define FALL_TIME(LEVEL) (MAX((1000 - (LEVEL - 1) * 80), 40))

#define SCORE_SINGLE(LEVEL) (100 * LEVEL)
#define SCORE_DOUBLE(LEVEL) (300 * LEVEL)
#define SCORE_MINI_TSPIN_DOUBLE(LEVEL) (400 * LEVEL)
#define SCORE_TRIPLE(LEVEL) (500 * LEVEL)
#define SCORE_B2B_MINI_TSPIN_DOUBLE(LEVEL) (600 * LEVEL)
#define SCORE_TETRIS(LEVEL) (800 * LEVEL)
#define SCORE_TSPIN_SINGLE(LEVEL) (SCORE_TETRIS(LEVEL))
#define SCORE_B2B_TETRIS(LEVEL) (1200 * LEVEL)
#define SCORE_B2B_TSPIN_SINGLE(LEVEL) (SCORE_B2B_TETRIS(LEVEL))
#define SCORE_TSPIN_DOUBLE(LEVEL) (SCORE_B2B_TETRIS(LEVEL))
#define SCORE_TSPIN_TRIPLE(LEVEL) (1600 * LEVEL)
#define SCORE_B2B_TSPIN_DOUBLE(LEVEL) (1800 * LEVEL)
#define SCORE_B2B_TSPIN_TRIPLE(LEVEL) (2400 * LEVEL)

typedef enum {
    BROKEN,
    TETRIS,
    TSPIN_SINGLE,
    TSPIN_DOUBLE,
    TSPIN_TRIPPLE,
    TSPIN_DOUBLE_MINI,
} b2b_t;

#define FIELD_POS_X (55)
#define FIELD_POS_Y (13)

/**
 * @param red: 0b000 - 0b111
 * @param green: 0b000 - 0b111
 * @param blue: 0b000 - 0b111
 * Generates a port config given the 3 colors.
 */
#define COLOR(red, green, blue)                                                \
    (uint16_t)(((red & 0b100) >> 1) | ((red & 0b010) << 9) |                   \
               ((red & 0b001) << 11) | ((green & 0b100) << 10) |               \
               ((green & 0b010) << 12) | ((green & 0b001) << 14) |             \
               ((blue & 0b100) << 5) | ((blue & 0b010) << 5) |                 \
               ((blue & 0b001) << 5))

#define PALETTE_RED COLOR(7, 4, 5), COLOR(4, 1, 1), COLOR(3, 0, 1)
#define PALETTE_BLUE COLOR(4, 6, 7), COLOR(1, 3, 4), COLOR(0, 2, 4)
#define PALETTE_GREEN COLOR(4, 6, 4), COLOR(1, 4, 2), COLOR(1, 2, 0)
#define PALETTE_MAGENTA COLOR(6, 4, 7), COLOR(4, 2, 4), COLOR(2, 1, 3)
#define PALETTE_ORANGE COLOR(6, 4, 7), COLOR(4, 2, 4), COLOR(2, 1, 3)
#define PALETTE_CYAN COLOR(6, 4, 7), COLOR(4, 2, 4), COLOR(2, 1, 3)

static const uint16_t PALETTES[][8] = {
    // Debug level
    [0] = {COLOR(0, 0, 0), COLOR(7, 7, 7), COLOR(7, 0, 0), COLOR(0, 7, 0),
           COLOR(0, 0, 7), COLOR(7, 7, 0), COLOR(7, 0, 7), COLOR(0, 7, 7)},
    // Red & Blue
    [1] = {COLOR(0, 0, 0), COLOR(7, 7, 7), PALETTE_RED, PALETTE_BLUE},
    // Green & Magenta
    [2] = {COLOR(0, 0, 0), COLOR(7, 7, 7), PALETTE_GREEN, PALETTE_MAGENTA},
    // Orange & Cyan
};

#endif // TETRIS_H
