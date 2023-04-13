#include "tetris.h"
#include "field.h"
#include "gfx.h"
#include "tetromino.h"
#include "timer.h"
#include <gpu.h>
#include <mes.h>
#include <rng.h>
#include <stdint.h>

uint8_t level;

uint8_t start(void) {
    gpu_blank(FRONT_BUFFER, 0x00);
    level = 1;
    rng_init();
    tetromino_t queue[5];
    for (uint8_t i = 0; i < 5; ++i) {
        queue[i] = tetromino_random();
    }

    field_t field = field_create();
    gpu_update_palette(PALETTES[level]);

    bool game_over = false;

    while (!game_over) {
	tetromino_t tet = tetromino_random();
	coord_t pos = POS(FIELD_X_SPAWN, FIELD_Y_SPAWN);
        while (field_try_draw_tetromino(&field, pos, &tet) && pos.y != 0) {
            field_draw_tetromino(&field, pos, &tet);
            gpu_send_buf(FRONT_BUFFER, field.size.x,
                         field.size.y - FIELD_OBSCURE, 0, 0,
                         (field.pixels + FIELD_OBSCURED_BYTES));
	    gpu_wait_for_next_ready();
	    timer_block_ms(120);
            field_clear_tetromino(&field, pos, &tet);
            pos.y--;
        }

        if (pos.y >= FIELD_Y_SPAWN) {
            game_over = true;
            break;
        }
        if (pos.y > 0)
            pos.y++;
        field_draw_tetromino(&field, pos, &tet);
    }

    if (game_over) {
        gpu_print_text(FRONT_BUFFER, 50, 50, 1, 0, "GAME OVER");
    }

    while (true)
        ;

    buffer_destory(&field);
}
