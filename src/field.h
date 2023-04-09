#ifndef FIELD_H
#define FIELD_H

#include "gfx.h"
#include "tetromino.h"
#include <stdint.h>

/*
 * 19|
 * 18|
 * 17|
 * 16|
 * 15|
 * 14|
 * 13|
 * 12|
 * 11|
 * 10|
 * 9 |
 * 8 |
 * 7 |
 * 6 |
 * 5 |
 * 4 |
 * 3 |
 * 2 |
 * 1 |
 * 0 |____________________
 *    0 1 2 3 4 5 6 7 8 9
 */

#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define FIELD_RESOLUTION 5

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
               ((blue & 0b100) << 5) | ((blue & 0b010) << 7) |                 \
               ((blue & 0b001) << 9))


static const uint16_t PALETTES[][8] = {
    // LEVEL 1: RED & BLUE
    {COLOR(0, 0, 0), COLOR(7, 7, 7), COLOR(7, 4, 5), COLOR(4, 1, 1),
     COLOR(3, 0, 1), COLOR(4, 6, 7), COLOR(1, 3, 4), COLOR(0, 2, 4)},
};

typedef buffer_t field_t;

field_t field_create();

void field_draw_tetromino(field_t *field, coord_t pos, tetromino_t *tet);

void field_draw_tile(field_t *field, coord_t pos, tile_t tile);

bool field_try_draw_tetromino(field_t *field, coord_t pos, tetromino_t *tet);

bool field_is_air(field_t *field, coord_t pos);

void field_clear_tile(field_t *field, coord_t pos);

uint8_t field_clear_lines(field_t *field);

// deg = 1  -> clockwise
// deg = -1 -> counter-clockwise
// deg = 2  -> 180°
vec2i8 tetromino_rotate(tetromino_t *tet, coord_t pos, field_t *field,
                        int8_t deg);


#endif // FIELD_H
