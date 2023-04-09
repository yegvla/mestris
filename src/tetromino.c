#include "tetromino.h"
#include "TILE_AIR.m3if.asset"
#include "TILE_BORDER.m3if.asset"
#include "TILE_FILLED.m3if.asset"
#include "TILE_GARBAGE.m3if.asset"
#include "TILE_GHOST.m3if.asset"
#include "field.h"
#include "gfx.h"
#include "tetris.h"
#include <stdint.h>


tetromino_t tetromino_create(shape_t shape, texture_t texture, color_t color) {
    return (tetromino_t){.shape = shape,
                         .rotation = DEG_0,
                         .style = {
                             .texture = texture,
                             .color = color,
                         }};
}

vec2i8 tetromino_rotate(tetromino_t *tet, coord_t pos, field_t *field,
                        int8_t deg) {
    rotation_t original_rot = tet->rotation;
    rotation_t new_rot = (tet->rotation + deg) & 0b11;
    const vec2i8(*offset_data)[4][5];
    switch (tet->shape) {
    case I:
        offset_data = &OFFSET_DATA_I;
        break;
    case J:
    case L:
    case S:
    case T:
    case Z:
        offset_data = &OFFSET_DATA_JLSTZ;
        break;
    case O:
        tet->rotation = new_rot;
        return OFFSET_DATA_O[tet->rotation];
    }

    vec2i8 tests[5] = {
        VEC2_SUB((*offset_data)[original_rot][0], (*offset_data)[new_rot][0]),
        VEC2_SUB((*offset_data)[original_rot][1], (*offset_data)[new_rot][1]),
        VEC2_SUB((*offset_data)[original_rot][2], (*offset_data)[new_rot][2]),
        VEC2_SUB((*offset_data)[original_rot][3], (*offset_data)[new_rot][3]),
        VEC2_SUB((*offset_data)[original_rot][4], (*offset_data)[new_rot][4]),
    };

    uint8_t kick = 0;
    bool has_room = false;
    for (; kick < 5; ++kick) {
        if (!field_try_draw_tetromino(field, VEC2_ADD(pos, tests[kick]), tet)) {
            continue;
        } else {
            has_room = true;
            break;
        }
    }
    if (has_room) {
        tet->rotation = new_rot;
        return tests[kick];
    } else {
        return (vec2i8){0, 0};
    }
}

const pixel_t *texture_get_pixles(texture_t texture) {
    switch (texture) {
    case AIR:
        return ASSET_TILE_AIR_M3IF;
    case FILLED:
        return ASSET_TILE_FILLED_M3IF;
    case BORDER:
        return ASSET_TILE_BORDER_M3IF;
    case GHOST:
        return ASSET_TILE_GHOST_M3IF;
    case GARBAGE:
        return ASSET_TILE_GARBAGE_M3IF;
    }
}

color_t pixel_apply_color(pixel_t pixel, color_t color) {
    // Textures are drawn with the A-color, if we need to draw a
    // B-Color tile then just map all A-colors in the tile to
    // B-colors, thus (+3).
    if (color == COLOR_B) {
        switch (pixel) {
        case COLOR_A1:
        case COLOR_A2:
        case COLOR_A3:
            return pixel + 3;
        }
    }
    return pixel;
}
