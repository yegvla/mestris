#include "tetris.h"
#include "TEXT_HOLD.m3if.asset"
#include "TEXT_LEVEL.m3if.asset"
#include "TEXT_NEXT.m3if.asset"
#include "TEXT_SCORE.m3if.asset"
#include "field.h"
#include "gfx.h"
#include "tetromino.h"
#include <gpu.h>
#include <input.h>
#include <mes.h>
#include <rng.h>
#include <stdint.h>
#include <timer.h>

uint8_t level;

#define CAN_MOVE(FRAMES_HELD, BTN)                                             \
    (FRAMES_HELD[BTN] == 1 ||                                                  \
     (FRAMES_HELD[BTN] > 15 && FRAMES_HELD[BTN] % 5 == 0) ||                   \
     (FRAMES_HELD[BTN] > 30 && FRAMES_HELD[BTN] % 2 == 0) ||                   \
     (FRAMES_HELD[BTN] > 60))

static void update_field(field_t *field) {
    gpu_send_buf(BACK_BUFFER, field->size.x, field->size.y - FIELD_OBSCURE,
                 FIELD_POS_X, FIELD_POS_Y,
                 (field->pixels + FIELD_OBSCURED_BYTES));
    gpu_swap_buf();
}

static void update_frames_held(uint32_t *frames_held) {
    for (uint8_t btn = 0; btn < 8; ++btn) {
        if (input_get_button(0, btn)) {
            frames_held[btn]++;
        } else {
            frames_held[btn] = 0;
        }
    }
}

static void next_level() {
    char txt[6];
    sprintf(txt, "%05i", ++level);
    gpu_print_text(FRONT_BUFFER, 9, 27, COLOR_FG, COLOR_BG, txt);
    gpu_print_text(BACK_BUFFER, 9, 27, COLOR_FG, COLOR_BG, txt);
    gpu_update_palette(PALETTES[level]);
    gpu_block_ack();
}

static void update_score(uint32_t score) {
    char txt[10];
    sprintf(txt, "%05i", score);
    gpu_print_text(FRONT_BUFFER, 9, 54, COLOR_FG, COLOR_BG, txt);
    gpu_print_text(BACK_BUFFER, 9, 54, COLOR_FG, COLOR_BG, txt);
    gpu_block_ack();
}

static void add_score(uint32_t *score, uint32_t add, uint8_t steps) {
    if (add > 0) {
        char txt[10];
        uint32_t tmp = *score;
        *score += add;
        for (uint8_t i = 0; i < steps; ++i) {
            tmp += add / steps;
            sprintf(txt, "%05i", tmp);
            gpu_print_text(FRONT_BUFFER, 9, 54, COLOR_FG, COLOR_BG, txt);
            gpu_block_ack();
            gpu_block_frame();
        }
        update_score(*score);
    }
}

typedef struct {
    bool rerender;
    bool lock;
    bool place;
    bool respawn;
} instructions_t;

static instructions_t process_controls(field_t *field, tetromino_t *tet,
                                       coord_t *pos, coord_t ghost_pos,
                                       tetromino_t *hold, bool has_respawned,
                                       tetromino_t *current_bag,
                                       uint8_t *bag_index,
                                       uint32_t *frames_held, uint32_t *score) {
    bool rerender = false;
    bool lock = false;
    bool place = false;
    bool respawn = false;
    update_frames_held(frames_held);

    // move left
    if (field_try_draw_tetromino(field, VEC2_SUB(*pos, POS(1, 0)), tet)) {
        if CAN_MOVE (frames_held, BUTTON_LEFT) {
            pos->x--;
            rerender = true;
        }
    }

    // move right
    if (field_try_draw_tetromino(field, VEC2_ADD(*pos, POS(1, 0)), tet)) {
        if CAN_MOVE (frames_held, BUTTON_RIGHT) {
            pos->x++;
            rerender = true;
        }
    }

    // move down
    if CAN_MOVE (frames_held, BUTTON_DOWN) {
        *score += 1;
        update_score(*score);
        if (field_try_draw_tetromino(field, VEC2_SUB(*pos, POS(0, 1)), tet)) {
            pos->y--;
        } else {
            lock = true;
            // for a better experience.
            frames_held[BUTTON_DOWN] = 0;
        }
        rerender = true;
    }

    int8_t rotate = DEG_0;

    // rotate right
    if (frames_held[BUTTON_A] == 1) {
        rotate = DEG_90;
    }

    // rotate left
    if (frames_held[BUTTON_B] == 1) {
        rotate = -DEG_90;
    }

    if (rotate != DEG_0) {
        vec2i8 offset = field_rotate_tetromino(field, *pos, tet, rotate);
        *pos = VEC2_ADD(*pos, offset);
        rerender = true;
    }

    // hard drop
    if (frames_held[BUTTON_UP] == 1) {
        add_score(score, (pos->y - ghost_pos.y) * 2, 2);
        *pos = ghost_pos;
        rerender = true;
        place = true;
    }

    if (frames_held[BUTTON_SELECT] == 1 && !has_respawned) {
        if (hold->shape == NONE) {
            *hold = *tet;
            *tet = current_bag[*bag_index++];
        } else {
            tetromino_t held = *hold;
            *hold = *tet;
            *tet = held;
        }
        respawn = true;
    }

    return (instructions_t){rerender, lock, place, respawn};
}

static void update_next(uint8_t bag_index, tetromino_t *current_bag,
                        tetromino_t *other_bag) {
    // TODO: coordinates
    buffer_t next_buf = buffer_alloc((screen_size_t){.x = 20, .y = 80});
    memset(next_buf.pixels, 0x00, BUFFER_SIZE(next_buf.size));
    uint8_t next_index = 0;
    bag_index++; // first one is the current one
    while (bag_index < 7 && next_index < 4) {
        tetromino_t *next = &current_bag[bag_index++];
        buffer_draw_tetromino(&next_buf, POS(1, 14 - (4 * next_index++)), next);
    }
    uint8_t tmp = 0;
    while (next_index < 4) {
        tetromino_t *next = &other_bag[tmp++];
        buffer_draw_tetromino(&next_buf, POS(1, 14 - (4 * next_index++)), next);
    }

    gpu_send_buf(FRONT_BUFFER, next_buf.size.x, next_buf.size.y, 125, 26,
                 next_buf.pixels);
    gpu_send_buf(BACK_BUFFER, next_buf.size.x, next_buf.size.y, 125, 26,
                 next_buf.pixels);
    gpu_block_ack();
    buffer_destory(&next_buf);
}

static void update_hold(tetromino_t *hold) {
    buffer_t hold_buf = buffer_alloc((screen_size_t){.x = 20, .y = 20});
    memset(hold_buf.pixels, 0x00, BUFFER_SIZE(hold_buf.size));
    if (hold->shape != NONE) {
        buffer_draw_tetromino(&hold_buf, POS(1, 1), hold);
    }
    gpu_send_buf(FRONT_BUFFER, hold_buf.size.x, hold_buf.size.y, 15, 80,
                 hold_buf.pixels);
    gpu_send_buf(BACK_BUFFER, hold_buf.size.x, hold_buf.size.y, 15, 80,
                 hold_buf.pixels);
    gpu_block_ack();
    buffer_destory(&hold_buf);
}

static void update_ghost(field_t *field, tetromino_t *original,
                         tetromino_t *ghost, coord_t pos, coord_t *ghost_pos) {
    *ghost = *original;
    ghost->style.texture = GHOST;

    *ghost_pos = pos;

    while (field_try_draw_tetromino(field, VEC2_SUB(*ghost_pos, POS(0, 1)),
                                    ghost)) {
        ghost_pos->y--;
    }
}

static void display_message(char *msg) {
    gpu_print_text(FRONT_BUFFER, 54, 4, COLOR_FG, COLOR_BG, msg);
}

uint8_t start(void) {
    buffer_t screen = buffer_alloc((screen_size_t){160, 120});
    memset(screen.pixels, 0x00, BUFFER_SIZE(screen.size));
    buffer_draw_line(&screen, 54, 12, 105, 12, COLOR_FG);
    buffer_draw_line(&screen, 105, 12, 105, 116, COLOR_FG);
    buffer_draw_line(&screen, 105, 115, 54, 115, COLOR_FG);
    buffer_draw_line(&screen, 54, 115, 54, 12, COLOR_FG);
    gpu_send_buf(FRONT_BUFFER, 160, 120, 0, 0, screen.pixels);
    gpu_send_buf(BACK_BUFFER, 160, 120, 0, 0, screen.pixels);
    gpu_send_buf(FRONT_BUFFER, 29, 8, 9, 12, (void *)ASSET_TEXT_LEVEL_M3IF);
    gpu_send_buf(BACK_BUFFER, 29, 8, 9, 12, (void *)ASSET_TEXT_LEVEL_M3IF);
    gpu_send_buf(FRONT_BUFFER, 29, 8, 9, 39, (void *)ASSET_TEXT_SCORE_M3IF);
    gpu_send_buf(BACK_BUFFER, 29, 8, 9, 39, (void *)ASSET_TEXT_SCORE_M3IF);
    gpu_send_buf(FRONT_BUFFER, 29, 8, 9, 66, (void *)ASSET_TEXT_HOLD_M3IF);
    gpu_send_buf(BACK_BUFFER, 29, 8, 9, 66, (void *)ASSET_TEXT_HOLD_M3IF);
    gpu_send_buf(FRONT_BUFFER, 29, 8, 120, 12, (void *)ASSET_TEXT_NEXT_M3IF);
    gpu_send_buf(BACK_BUFFER, 29, 8, 120, 12, (void *)ASSET_TEXT_NEXT_M3IF);
    buffer_destory(&screen);
    level = 0;
    next_level();
    uint32_t score = 0;
    update_score(score);
    uint32_t lines_cleared = 0;
    b2b_t last_b2b = BROKEN;

    rng_init();
    tetromino_t queue[5];
    for (uint8_t i = 0; i < 5; ++i) {
        queue[i] = tetromino_random();
    }

    gpu_update_palette(PALETTES[level]);

    field_t field = field_create();
    uint8_t bag_index = 0;
    tetromino_t bag_a[7];
    tetromino_t bag_b[7];
    tetromino_t *current_bag = bag_a;
    tetromino_t *other_bag = bag_b;
    tetromino_random_bag(bag_a);
    tetromino_random_bag(bag_b);
    tetromino_t hold = tetromino_create_empty();

    bool game_over = false;
    coord_t pos;
    coord_t ghost_pos;

    uint32_t frames_held[8] = {0};

    while (!game_over) {
        if (bag_index > 6) {
            tetromino_random_bag(current_bag);
            if (current_bag == bag_a) {
                current_bag = bag_b;
                other_bag = bag_a;
            } else {
                current_bag = bag_a;
                other_bag = bag_b;
            }
            bag_index = 0;
        }

        update_next(bag_index, current_bag, other_bag);

        tetromino_t *tet = &current_bag[bag_index++];
        tetromino_t ghost;
        bool has_respawned = false;

    respawn:
        pos = POS(FIELD_X_SPAWN, FIELD_Y_SPAWN);
        ghost_pos = pos;

        // TODO: is this even needed?
        if (!field_try_draw_tetromino(&field, pos, tet) &&
            field_try_draw_tetromino(&field, VEC2_ADD(pos, POS(0, 1)), tet)) {
            pos.y++;
        }

        if (!field_try_draw_tetromino(&field, pos, tet)) {
            game_over = true;
            break;
        }

        uint32_t lock_penalty = 0;
        while (
            field_try_draw_tetromino(&field, VEC2_SUB(pos, POS(0, 1)), tet)) {
        airborne:
            update_ghost(&field, tet, &ghost, pos, &ghost_pos);
            field_draw_tetromino(&field, ghost_pos, &ghost);
            field_draw_tetromino(&field, pos, tet);
            update_field(&field);
            field_clear_tetromino(&field, pos, tet);
            field_clear_tetromino(&field, ghost_pos, &ghost);
            uint32_t wait_until = timer_get_ms() + FALL_TIME(level);
            while (wait_until > timer_get_ms()) {
                if (!input_is_available(0)) {
                    continue;
                }

                instructions_t inst = process_controls(
                    &field, tet, &pos, ghost_pos, &hold, has_respawned,
                    current_bag, &bag_index, frames_held, &score);

                if (inst.respawn) {
		    update_hold(&hold);
                    has_respawned = true;
                    goto respawn;
                }

                if (inst.rerender) {
                    update_ghost(&field, tet, &ghost, pos, &ghost_pos);
                    field_draw_tetromino(&field, ghost_pos, &ghost);
                    field_draw_tetromino(&field, pos, tet);
                    update_field(&field);
                    field_clear_tetromino(&field, pos, tet);
                    field_clear_tetromino(&field, ghost_pos, &ghost);
                }

                if (inst.place) {
                    lock_penalty = LOCK_TIME;
                    break;
                }

                if (!field_try_draw_tetromino(&field, VEC2_SUB(pos, POS(0, 1)),
                                              tet)) {
                    break;
                }

                if (inst.lock) {
                    break;
                }

                if (lines_cleared >= level * 10) {
                    lines_cleared -= level * 10;
                    next_level();
                }

                gpu_block_ack();
                gpu_block_frame();
            }
            pos.y--;
        }

        if (!field_try_draw_tetromino(&field, pos, tet)) {
            pos.y++;
        }

        tet->style.color = color_get_light_var(tet->style.color);

        field_draw_tetromino(&field, pos, tet);
        update_field(&field);
        field_clear_tetromino(&field, pos, tet);

        uint32_t wait_until = timer_get_ms() + LOCK_TIME - lock_penalty;
        lock_penalty += 200;
        while (wait_until > timer_get_ms()) {
            instructions_t inst = process_controls(
                &field, tet, &pos, ghost_pos, &hold, has_respawned, current_bag,
                &bag_index, frames_held, &score);

            if (inst.respawn) {
		update_hold(&hold);
                has_respawned = true;
                goto respawn;
            }

            if (inst.rerender) {
                field_draw_tetromino(&field, pos, tet);
                update_field(&field);
                field_clear_tetromino(&field, pos, tet);
            }

            if (inst.lock || inst.place) {
                break;
            }

            if (field_try_draw_tetromino(&field, VEC2_SUB(pos, POS(0, 1)),
                                         tet)) {
                tet->style.color = color_get_normal_var(tet->style.color);
                goto airborne;
            }

            gpu_block_ack();
            gpu_block_frame();
        }

        tet->style.color = color_get_normal_var(tet->style.color);

        field_draw_tetromino(&field, pos, tet);

        uint8_t lines = field_clear_lines(&field);
        lines_cleared += lines;

        uint32_t reward = 0;

        switch (lines) {
        case 1:
            reward = SCORE_SINGLE(level);
            last_b2b = BROKEN;
            break;
        case 2:
            display_message("DOUBLE");
            reward = SCORE_DOUBLE(level);
            last_b2b = BROKEN;
            break;
        case 3:
            display_message("TRIPLE");
            reward = SCORE_TRIPLE(level);
            last_b2b = BROKEN;
            break;
        case 4:
            if (last_b2b == TETRIS) {
                display_message("B2B TETRIS");
                reward = SCORE_B2B_TETRIS(level);
            } else {
                display_message("TETRIS");
                reward = SCORE_TETRIS(level);
            }
            last_b2b = TETRIS;
            break;
        }

        add_score(&score, reward, 8);
        if (reward > SCORE_SINGLE(level)) {
            display_message("          ");
        }
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
    free(bag_a);
    free(bag_b);
    return CODE_EXIT;
}
