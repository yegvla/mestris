#include "tetris.h"
#include "field.h"
#include "tetromino.h"
#include "gfx.h"
#include <gpu.h>
#include <mes.h>

uint8_t level;

uint8_t start(void) {
    level = 1;
    field_t field = field_create();
    tetromino_t tet = tetromino_create(L, FILLED, COLOR_A);
    gpu_update_palette(PALETTES[level - 1]);
    field_draw_tetromino(&field, POS(FIELD_X_SPAWN, FIELD_Y_SPAWN), &tet);
    while (true) {
        gpu_send_buf(FRONT_BUFFER, field.size.x, field.size.y - FIELD_OBSCURE, 0, 0,
                     (field.pixels + FIELD_OBSCURED_BYTES));
    }
    buffer_destory(&field);
}
