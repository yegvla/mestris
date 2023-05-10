#ifndef FIELD_H
#define FIELD_H

#include "gfx.h"
#include "tetromino.h"
#include <stdint.h>

/*
 * Tetrominos should be spawned at 19.  If they can't spawn at 19,
 * they should spawn at 20.  If they can't spawn at 20 then the player
 * lost.
 *
 * 21|    NOT VISIBLE
 * 20|-------------------- <- only 2 pixels of
 * 19|                        20 are visible.
 * 18|      []
 * 17|  [][][]
 * 16|     ^ POS(2, 17)
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
#define FIELD_HEIGHT 22
#define FIELD_OBSCURE 8
#define FIELD_OBSCURED_BYTES                                                   \
    ((FIELD_OBSCURE * FIELD_RESOLUTION * FIELD_WIDTH * BUFFER_BPP) / 8)
#define FIELD_RESOLUTION 5

#define FIELD_Y_SPAWN 19
#define FIELD_X_SPAWN 4

typedef buffer_t field_t;

field_t field_create();

void field_draw_tetromino(field_t *field, coord_t pos, tetromino_t *tet);

void buffer_draw_tetromino(buffer_t *buf, coord_t pos, tetromino_t *tet);

void field_clear_tetromino(field_t *field, coord_t pos, tetromino_t *tet);

void field_draw_tile(field_t *field, coord_t pos, tile_t tile);

void buffer_draw_tile(buffer_t *buf, coord_t pos, tile_t tile);

bool field_try_draw_tetromino(field_t *field, coord_t pos, tetromino_t *tet);

bool field_is_air(field_t *field, coord_t pos);

void field_clear_tile(field_t *field, coord_t pos);

uint8_t field_clear_lines(field_t *field);

// deg = 1  -> clockwise
// deg = -1 -> counter-clockwise
// deg = 2  -> 180°
vec2i8 field_rotate_tetromino(field_t *field, coord_t pos, tetromino_t *tet,
                              int8_t deg);

#endif // FIELD_H
