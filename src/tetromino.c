#include "tetromino.h"
#include "TILE_AIR.m3if.asset"
#include "TILE_BORDER.m3if.asset"
#include "TILE_FILLED.m3if.asset"
#include "TILE_GARBAGE.m3if.asset"
#include "TILE_GHOST.m3if.asset"
#include "field.h"
#include "gfx.h"
#include "tetris.h"
#include <rng.h>
#include <stdint.h>

tetromino_t tetromino_create(shape_t shape, texture_t texture, color_t color) {
    return (tetromino_t){.shape = shape,
                         .rotation = DEG_0,
                         .style = {
                             .texture = texture,
                             .color = color,
                         }};
}

tetromino_t tetromino_create_empty(void) {
    return (tetromino_t){.shape = NONE,
                         .rotation = DEG_0,
                         .style = {
                             .texture = 0,
                             .color = 0,
                         }};
}

tetromino_t tetromino_random(void) {
    uint32_t random = rng_u32();
    return (tetromino_t){.rotation = DEG_0,
                         .shape = random % 7,
                         .style = {
                             .color = ((random >> 4) & 1),
                             .texture = 1 + ((random >> 5) & 1),
                         }};
}

void tetromino_random_bag(tetromino_t *bag) {
    static const uint8_t ALL_SHAPES =
        BIT(I) | BIT(J) | BIT(L) | BIT(O) | BIT(S) | BIT(T) | BIT(Z);
    uint8_t bag_index = 0;
    uint8_t shapes_used = 0x00;
    while (shapes_used != ALL_SHAPES) {
        tetromino_t pick = tetromino_random();
        if (shapes_used & BIT(pick.shape)) {
            continue;
        }
        bag[bag_index++] = pick;
        shapes_used |= BIT(pick.shape);
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
    default:
        return ASSET_TILE_GARBAGE_M3IF;
    }
}

color_t pixel_apply_color(pixel_t pixel, color_t color) {
    // draw with a lighter color
    if (color == COLOR_A_LIGHT || color == COLOR_B_LIGHT) {
        switch (pixel) {
        case COLOR_A2:
        case COLOR_A3:
            pixel -= 1;
        }
    }
    // Textures are drawn with the A-color, if we need to draw a
    // B-Color tile then just map all A-colors in the tile to
    // B-colors, thus (+3).
    if (color == COLOR_B || color == COLOR_B_LIGHT) {
        switch (pixel) {
        case COLOR_A1:
        case COLOR_A2:
        case COLOR_A3:
            return pixel + 3;
        }
    }

    return pixel;
}

color_t color_get_light_var(color_t color) {
    switch (color) {
    case COLOR_A:
    case COLOR_B:
        return color + 2;
    default:
        return color;
    }
}

color_t color_get_normal_var(color_t color) {
    switch (color) {
    case COLOR_A_LIGHT:
    case COLOR_B_LIGHT:
        return color - 2;
    default:
        return color;
    }
}
