#include "field.h"
#include "gfx.h"
#include "gpu.h"
#include "tetris.h"
#include "tetromino.h"
#include "timer.h"
#include <stdint.h>

field_t field_create() {
    field_t field = buffer_alloc((screen_size_t){
        .x = FIELD_WIDTH * FIELD_RESOLUTION,
        .y = FIELD_HEIGHT * FIELD_RESOLUTION,
    });
    memset(field.pixels, 0x00, BUFFER_SIZE(field.size));
    return field;
}

void field_draw_tetromino(field_t *field, coord_t pos, tetromino_t *tet) {
    const vec2i8 *tiles = SHAPES[tet->shape][tet->rotation];
    for (uint8_t i = 0; i < 4; ++i) {
        vec2i8 tile_pos = tiles[i];
        field_draw_tile(field, VEC2_ADD(pos, tile_pos), tet->style);
    }
}

void buffer_draw_tetromino(buffer_t *buf, coord_t pos, tetromino_t *tet) {
    const vec2i8 *tiles = SHAPES[tet->shape][tet->rotation];
    for (uint8_t i = 0; i < 4; ++i) {
        vec2i8 tile_pos = tiles[i];
        buffer_draw_tile(buf, VEC2_ADD(pos, tile_pos), tet->style);
    }
}

void field_clear_tetromino(field_t *field, coord_t pos, tetromino_t *tet) {
    const vec2i8 *tiles = SHAPES[tet->shape][tet->rotation];
    for (uint8_t i = 0; i < 4; ++i) {
        vec2i8 tile_pos = tiles[i];
        field_clear_tile(field, VEC2_ADD(pos, tile_pos));
    }
}

bool field_try_draw_tetromino(field_t *field, coord_t pos, tetromino_t *tet) {
    const vec2i8 *tiles = SHAPES[tet->shape][tet->rotation];
    for (uint8_t i = 0; i < 4; ++i) {
        vec2i8 tile_pos = tiles[i];
        coord_t target_pos = VEC2_ADD(pos, tile_pos);
        if (target_pos.x >= FIELD_WIDTH || target_pos.y >= FIELD_HEIGHT) {
            return false;
        }
        if (!field_is_air(field, target_pos)) {
            return false;
        }
    }
    return true;
}

void field_draw_tile(field_t *field, coord_t pos, tile_t tile) {
    pos.y = FIELD_HEIGHT - 1 - pos.y;
    coord_t px_pos = VEC2_SCALE(pos, FIELD_RESOLUTION);
    const pixel_t *pixels = texture_get_pixles(tile.texture);
    const buffer_t texture = (buffer_t){
        .pixels = (void *)pixels,
        .size = {FIELD_RESOLUTION, FIELD_RESOLUTION},
    };
    for (uint8_t x = 0; x < FIELD_RESOLUTION; ++x) {
        for (uint8_t y = 0; y < FIELD_RESOLUTION; ++y) {
            pixel_t pixel = pixel_apply_color(
                buffer_get_pixel(&texture, POS(x, y)), tile.color);
            buffer_set_pixel(field, VEC2_ADD(px_pos, POS(x, y)), pixel);
        }
    }
}

void buffer_draw_tile(buffer_t *buf, coord_t pos, tile_t tile) {
    pos.y = (buf->size.y / FIELD_RESOLUTION) - 1 - pos.y;
    coord_t px_pos = VEC2_SCALE(pos, FIELD_RESOLUTION);
    const pixel_t *pixels = texture_get_pixles(tile.texture);
    const buffer_t texture = (buffer_t){
        .pixels = (void *)pixels,
        .size = {FIELD_RESOLUTION, FIELD_RESOLUTION},
    };
    for (uint8_t x = 0; x < FIELD_RESOLUTION; ++x) {
        for (uint8_t y = 0; y < FIELD_RESOLUTION; ++y) {
            pixel_t pixel = pixel_apply_color(
                buffer_get_pixel(&texture, POS(x, y)), tile.color);
            buffer_set_pixel(buf, VEC2_ADD(px_pos, POS(x, y)), pixel);
        }
    }
}

void field_clear_tile(field_t *field, coord_t pos) {
    pos.y = FIELD_HEIGHT - 1 - pos.y;
    coord_t px_pos = VEC2_SCALE(pos, FIELD_RESOLUTION);
    for (uint8_t x = 0; x < FIELD_RESOLUTION; ++x) {
        for (uint8_t y = 0; y < FIELD_RESOLUTION; ++y) {
            buffer_set_pixel(field, VEC2_ADD(px_pos, POS(x, y)), COLOR_BG);
        }
    }
}

bool field_is_air(field_t *field, coord_t pos) {
    pos.y = FIELD_HEIGHT - 1 - pos.y;
    coord_t px_pos = VEC2_SCALE(pos, FIELD_RESOLUTION);
    px_pos.x += 2; // hack for not detecting placement prediction.
    return buffer_get_pixel(field, px_pos) == COLOR_BG;
}

uint8_t field_clear_lines(field_t *field) {
    uint8_t counter = 0;
    uint8_t cleared_lines[4] = {0xff, 0xff, 0xff, 0xff};

    // TODO: not very efficient...  You could do every thing with
    // just the first chain of for loops... but because I want a
    // nice looking animation I need to compensate for
    // performance.
    for (uint8_t y = 0; y < FIELD_HEIGHT; ++y) {
        bool is_full = true;
        for (uint8_t x = 0; x < FIELD_WIDTH; ++x) {
            if (field_is_air(field, POS(x, y))) {
                is_full = false;
                break;
            }
        }

        if (is_full) {
            cleared_lines[counter++] = y;

            for (uint8_t x = 0; x < FIELD_WIDTH; ++x) {
                field_clear_tile(field, POS(x, y));
            }

            for (uint8_t px_y = 0; px_y < FIELD_RESOLUTION; ++px_y) {
                for (uint8_t px_x = 0; px_x < FIELD_WIDTH * FIELD_RESOLUTION;
                     ++px_x) {

                    uint8_t move_y_px =
                        (FIELD_HEIGHT - 1 - y) * FIELD_RESOLUTION + px_y;

                    buffer_set_pixel(field, POS(px_x, move_y_px), COLOR_BG);
                }
            }
        }
    }

    // check if any lines were actually cleared.
    if (cleared_lines[0] != 0xff) {
        uint8_t cursor = cleared_lines[0] + 1;
        for (uint8_t line = cleared_lines[0]; line < FIELD_HEIGHT; ++line) {
            bool is_line_empty;
            do {
                is_line_empty = false;
                for (uint8_t j = 0; j < 4; ++j) {
                    if (cleared_lines[j] == cursor) {
                        is_line_empty = true;
                        cursor++;
                        break;
                    }
                }
            } while (is_line_empty && cursor < FIELD_HEIGHT);

            if (cursor >= FIELD_HEIGHT) {
                cursor = FIELD_HEIGHT - 1;
            }

            bool has_changes = false;
            for (int8_t px_y = FIELD_RESOLUTION - 1; px_y >= 0; --px_y) {
                for (uint8_t px_x = 0; px_x < FIELD_WIDTH * FIELD_RESOLUTION;
                     ++px_x) {
                    coord_t origin = POS(
                        px_x,
                        (FIELD_HEIGHT - 1 - cursor) * FIELD_RESOLUTION + px_y);
                    coord_t dest =
                        POS(px_x, (FIELD_HEIGHT - 1 - line) * FIELD_RESOLUTION +
                                      px_y);
                    pixel_t pixel = buffer_get_pixel(field, origin);
                    if (pixel != COLOR_BG ||
                        buffer_get_pixel(field, dest) != COLOR_BG) {
                        has_changes = true;
                    }
                    buffer_set_pixel(field, dest, pixel);
                    buffer_set_pixel(field, origin, COLOR_BG);
                }
            }
            cursor++;
            if (has_changes) {
                gpu_block_ack();
                gpu_block_frame();
                gpu_send_buf(FRONT_BUFFER, field->size.x,
                             field->size.y - FIELD_OBSCURE, FIELD_POS_X,
                             FIELD_POS_Y,
                             (field->pixels + FIELD_OBSCURED_BYTES));
            }
        }
    }
    return counter;
}

vec2i8 field_rotate_tetromino(field_t *field, coord_t pos, tetromino_t *tet,
                              int8_t deg) {
    rotation_t original_rot = tet->rotation;
    rotation_t new_rot = (tet->rotation + deg) & 0b11;
    tet->rotation = new_rot;
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
        return VEC2_SUB(OFFSET_DATA_O[original_rot], OFFSET_DATA_O[new_rot]);
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
        if (field_try_draw_tetromino(field, VEC2_ADD(pos, tests[kick]), tet)) {
            has_room = true;
            break;
        }
    }
    if (has_room) {
        return tests[kick];
    } else {
        tet->rotation = original_rot;
        return (vec2i8){0, 0};
    }
}
