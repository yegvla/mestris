#include "tetris.h"
#include "field.h"
#include "gfx.h"
#include "tetromino.h"
#include <gpu.h>
#include <mes.h>

uint8_t level;

uint8_t start(void) {
    level = 1;
    field_t field = field_create();
    tetromino_t tet = tetromino_create(L, FILLED, COLOR_A);
    gpu_update_palette(PALETTES[level]);
    field_draw_tetromino(&field, POS(5, 5), &tet);
    while (true) {
        gpu_send_buf(FRONT_BUFFER, field.size.x, field.size.y, 0, 0,
                     field.pixels);
    }
    buffer_destory(&field);
}
