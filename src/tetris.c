#include "tetris.h"
#include "field.h"
#include "gfx.h"
#include "input.h"
#include "tetromino.h"
#include "timer.h"
#include <gpu.h>
#include <mes.h>
#include <rng.h>
#include <stdint.h>

uint8_t level;

#define FALL_TIME (1000 - 10 * level)
#define CAN_MOVE(FRAMES_HELD, BTN)                                             \
    (FRAMES_HELD[BTN] == 1 ||                                                  \
     (FRAMES_HELD[BTN] > 15 && FRAMES_HELD[BTN] % 5 == 0) ||                   \
     (FRAMES_HELD[BTN] > 30 && FRAMES_HELD[BTN] % 2 == 0) ||                   \
     (FRAMES_HELD[BTN] > 60))

static void update_field(field_t *field) {
    gpu_send_buf(BACK_BUFFER, field->size.x, field->size.y - FIELD_OBSCURE, 0,
                 0, (field->pixels + FIELD_OBSCURED_BYTES));
    gpu_swap_buf();
}

uint8_t start(void) {
    gpu_blank(FRONT_BUFFER, 0x00);
    gpu_blank(BACK_BUFFER, 0x00);
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
        uint32_t frames_held[8] = {0};
        while (field_try_draw_tetromino(&field, pos, tet) && pos.y != 0) {
            field_draw_tetromino(&field, pos, tet);
            update_field(&field);
            field_clear_tetromino(&field, pos, tet);
            uint32_t wait_until = timer_get_ms() + FALL_TIME;
            while (wait_until > timer_get_ms()) {
                if (!input_is_available(0)) {
                    continue;
                }
                bool rerender = false;
                for (uint8_t btn = 0; btn < 8; ++btn) {
                    if (input_get_button(0, btn)) {
                        frames_held[btn]++;
                    } else {
                        frames_held[btn] = 0;
                    }
                }

                // move left
                if (field_try_draw_tetromino(&field, VEC2_SUB(pos, POS(1, 0)),
                                             tet)) {
                    if CAN_MOVE (frames_held, BUTTON_LEFT) {
                        pos.x--;
                        rerender = true;
                    }
                }

                // move right
                if (field_try_draw_tetromino(&field, VEC2_ADD(pos, POS(1, 0)),
                                             tet)) {
                    if CAN_MOVE (frames_held, BUTTON_RIGHT) {
                        pos.x++;
                        rerender = true;
                    }
                }

                // move down
                if (field_try_draw_tetromino(&field, VEC2_SUB(pos, POS(0, 1)),
                                             tet)) {
                    if CAN_MOVE (frames_held, BUTTON_DOWN) {
                        pos.y--;
                        rerender = true;
                    }
                }

                int8_t rotate = 0;

                // rotate right
                if (frames_held[BUTTON_A] == 1) {
                    rotate = 1;
                }

                // rotate left
                if (frames_held[BUTTON_B] == 1) {
                    rotate = -1;
                }

                if (rotate != 0) {
                    vec2i8 offset =
                        field_rotate_tetromino(&field, pos, tet, rotate);
                    pos = VEC2_ADD(pos, offset);
                    if (offset.y > 0) {
                        wait_until = timer_get_ms() + FALL_TIME;
                    }
                    rerender = true;
                }

                if (rerender) {
                    field_draw_tetromino(&field, pos, tet);
                    update_field(&field);
                    field_clear_tetromino(&field, pos, tet);
                }
                gpu_block_ack();
                gpu_block_frame();
            }
            pos.y--;
        }

        if (!field_try_draw_tetromino(&field, pos, tet)) {
            pos.y++;
        }

        field_draw_tetromino(&field, pos, tet);
        update_field(&field);

        /* if (pos.y >= FIELD_Y_SPAWN) { */
        /*     game_over = true; */
        /*     break; */
        /* } */
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
