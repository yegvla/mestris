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

#define LOCK_TIME (1000)
#define FALL_TIME(LEVEL) (1000 - (LEVEL - 1) * 10)

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

static const uint16_t PALETTES[][8] = {
    // Debug level
    [0] = {COLOR(0, 0, 0), COLOR(7, 7, 7), COLOR(7, 0, 0), COLOR(0, 7, 0),
           COLOR(0, 0, 7), COLOR(7, 7, 0), COLOR(7, 0, 7), COLOR(0, 7, 7)},
    // Red & Blue
    [1] = {COLOR(0, 0, 0), COLOR(7, 7, 7), PALETTE_RED, PALETTE_BLUE},
    // Green & Magenta
    [2] = {COLOR(0, 0, 0), COLOR(7, 7, 7), PALETTE_GREEN, PALETTE_MAGENTA},
};

#endif // TETRIS_H
