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

    gpu_update_palette(PALETTES[level]);

    field_t field = field_create();
    int8_t bag_index = 0;
    tetromino_t *bag = (tetromino_t *)malloc(7 * sizeof(tetromino_t));
    tetromino_random_bag(bag);
    bool game_over = false;

    while (!game_over) {
        if (bag_index > 6) {
            tetromino_random_bag(bag);
            bag_index = 0;
        }
        tetromino_t *tet = &bag[bag_index++];
        coord_t pos = POS(FIELD_X_SPAWN, FIELD_Y_SPAWN);
        while (field_try_draw_tetromino(&field, pos, tet) && pos.y != 0) {
            field_draw_tetromino(&field, pos, tet);
            gpu_send_buf(FRONT_BUFFER, field.size.x,
                         field.size.y - FIELD_OBSCURE, 0, 0,
                         (field.pixels + FIELD_OBSCURED_BYTES));
            timer_block_ms(120);
            gpu_block_ack();
            gpu_block_frame();
            field_clear_tetromino(&field, pos, tet);
            pos.y--;
        }

        if (pos.y >= FIELD_Y_SPAWN) {
            game_over = true;
            break;
        }
        if (!field_try_draw_tetromino(&field, pos, tet)) {
            pos.y++;
        }
        field_draw_tetromino(&field, pos, tet);
        timer_block_ms(120);
    }

    if (game_over) {
        gpu_print_text(FRONT_BUFFER, 50, 50, 1, 0, "GAME OVER");
        timer_block_ms(1000);
        goto tetris_quit;
    }

    while (true)
        ;

tetris_quit:
    buffer_destory(&field);
    free(bag);
    return CODE_EXIT;
}
