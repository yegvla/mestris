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

void destroy_game(game *ctx) {
    free(ctx->field.pixels);
    free(ctx->bag_a);
    free(ctx->bag_b);
}

static void update_field(field *field) {
    gpu_send_buf(BACK_BUFFER, field->size.x, field->size.y - FIELD_OBSCURE, FIELD_POS_X,
                 FIELD_POS_Y, (field->pixels + FIELD_OBSCURED_BYTES));
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

static void next_level(game *ctx) {
    char txt[6];
    sprintf(txt, "%05i", ++ctx->level);
    gpu_print_text(FRONT_BUFFER, 9, 27, COLOR_FG, COLOR_BG, txt);
    gpu_print_text(BACK_BUFFER, 9, 27, COLOR_FG, COLOR_BG, txt);
    gpu_update_palette(PALETTES[ctx->level]);
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

static instructions process_controls(game *ctx) {
    bool rerender = false;
    bool lock = false;
    bool place = false;
    bool respawn = false;
    update_frames_held(ctx->frames_held);

    // Move left
    if (field_try_draw_tetromino(&ctx->field, VEC2_SUB(ctx->pos, POS(1, 0)), ctx->tet)) {
        if CAN_MOVE (ctx->frames_held, BUTTON_LEFT) {
            ctx->pos.x--;
            rerender = true;
        }
    }

    // Move right
    if (field_try_draw_tetromino(&ctx->field, VEC2_ADD(ctx->pos, POS(1, 0)), ctx->tet)) {
        if CAN_MOVE (ctx->frames_held, BUTTON_RIGHT) {
            ctx->pos.x++;
            rerender = true;
        }
    }

    // Move down
    if CAN_MOVE (ctx->frames_held, BUTTON_DOWN) {
        ctx->score += 1;
        update_score(ctx->score);
        if (field_try_draw_tetromino(&ctx->field, VEC2_SUB(ctx->pos, POS(0, 1)), ctx->tet)) {
            ctx->pos.y--;
        } else {
            lock = true;
            // for a better experience.
            ctx->frames_held[BUTTON_DOWN] = 0;
        }
        rerender = true;
    }

    int8_t rotate = DEG_0;

    // Rotate right
    if (ctx->frames_held[BUTTON_A] == 1) {
        rotate = DEG_90;
    }

    // Rotate left
    if (ctx->frames_held[BUTTON_B] == 1) {
        rotate = -DEG_90;
    }

    // Rotate if a rotation was set.
    if (rotate != DEG_0) {
        vec2i8 offset = field_rotate_tetromino(&ctx->field, ctx->pos, ctx->tet, rotate);
        ctx->pos = VEC2_ADD(ctx->pos, offset);
        rerender = true;
    }

    // Hard drop
    if (ctx->frames_held[BUTTON_UP] == 1) {
        add_score(&ctx->score, (ctx->pos.y - ctx->ghost_pos.y) * 2, 2);
        ctx->pos = ctx->ghost_pos;
        rerender = true;
        place = true;
    }

    if (ctx->frames_held[BUTTON_SELECT] == 1 && !ctx->has_respawned) {
        if (ctx->hold.shape == NONE) {
            ctx->hold = *(ctx->tet);
            *(ctx->tet) = ctx->current_bag[ctx->bag_index++];
        } else {
            tetromino held = ctx->hold;
            ctx->hold = *(ctx->tet);
            *ctx->tet = held;
        }
        respawn = true;
    }

    return (instructions){rerender, lock, place, respawn};
}

static void update_next_preview(game *ctx) {
    // TODO: coordinates
    buffer next_buf = buffer_alloc((screen_size){.x = 20, .y = 80});
    memset(next_buf.pixels, 0x00, BUFFER_SIZE(next_buf.size));
    uint8_t next_index = 0;
    uint8_t bag_index = ctx->bag_index;
    bag_index++; // first one is the current one
    while (bag_index < 7 && next_index < 4) {
        tetromino *next = &ctx->current_bag[bag_index++];
        buffer_draw_tetromino(&next_buf, POS(1, 14 - (4 * next_index++)), next);
    }
    bag_index = 0;
    while (next_index < 4) {
        tetromino *next = &ctx->other_bag[bag_index++];
        buffer_draw_tetromino(&next_buf, POS(1, 14 - (4 * next_index++)), next);
    }

    gpu_send_buf(FRONT_BUFFER, next_buf.size.x, next_buf.size.y, 125, 26, next_buf.pixels);
    gpu_send_buf(BACK_BUFFER, next_buf.size.x, next_buf.size.y, 125, 26, next_buf.pixels);
    gpu_block_ack();
    buffer_destory(&next_buf);
}

static void render_hold_piece(tetromino *hold) {
    buffer hold_buf = buffer_alloc((screen_size){.x = 20, .y = 20});
    memset(hold_buf.pixels, 0x00, BUFFER_SIZE(hold_buf.size));
    if (hold->shape != NONE) {
        buffer_draw_tetromino(&hold_buf, POS(1, 1), hold);
    }
    gpu_send_buf(FRONT_BUFFER, hold_buf.size.x, hold_buf.size.y, 15, 80, hold_buf.pixels);
    gpu_send_buf(BACK_BUFFER, hold_buf.size.x, hold_buf.size.y, 15, 80, hold_buf.pixels);
    gpu_block_ack();
    buffer_destory(&hold_buf);
}

static void update_ghost_piece(game *ctx) {
    ctx->ghost = *(ctx->tet);
    ctx->ghost.style.texture = GHOST;

    ctx->ghost_pos = ctx->pos;

    while (
        field_try_draw_tetromino(&ctx->field, VEC2_SUB(ctx->ghost_pos, POS(0, 1)), &ctx->ghost)) {
        ctx->ghost_pos.y--;
    }
}

static void display_message(char *msg) {
    gpu_print_text(FRONT_BUFFER, 54, 4, COLOR_FG, COLOR_BG, msg);
}

uint8_t start(void) {
    buffer screen = buffer_alloc((screen_size){160, 120});
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

    rng_init();

    // Create game context.
    game ctx = (game){
        .level = 0,
        .field = field_create(),
        .tet = nullptr,
        .pos = POS(0, 0),
        .ghost_pos = POS(0, 0),
        .hold = tetromino_create_empty(),
        .has_respawned = false,
        .bag_a = malloc(sizeof(tetromino) * 7),
        .bag_b = malloc(sizeof(tetromino) * 7),
        .current_bag = nullptr,
        .other_bag = nullptr,
        .bag_index = 0,
        .frames_held = {0},
        .score = 0,
        .lines_cleared = 0,
        .last_b2b = BROKEN,
        .lock_penalty = 0,
        .game_over = false,
    };

    // Populate bags.
    tetromino_random_bag(ctx.bag_a);
    tetromino_random_bag(ctx.bag_b);

    // Define "current" and "other" bags.
    ctx.current_bag = ctx.bag_a;
    ctx.other_bag = ctx.bag_b;

    // Display initaial score
    update_score(ctx.score);

    // Start at level 1
    next_level(&ctx);

    while (!ctx.game_over) {
        // If we reach the end of a bag we switch the current bag to
        // the other bag, we need 2 bags because we display up to 4
        // next pieces.
        if (ctx.bag_index > 6) {
            tetromino_random_bag(ctx.current_bag);
            if (ctx.current_bag == ctx.bag_b) {
                ctx.current_bag = ctx.bag_b;
                ctx.other_bag = ctx.bag_a;
            } else {
                ctx.current_bag = ctx.bag_a;
                ctx.other_bag = ctx.bag_b;
            }
            ctx.bag_index = 0;
        }

        update_next_preview(&ctx);
        ctx.tet = &ctx.current_bag[ctx.bag_index++];
	ctx.has_respawned = false;

    respawn:
        ctx.pos = POS(FIELD_X_SPAWN, FIELD_Y_SPAWN);
        ctx.ghost_pos = ctx.pos;

        // TODO: is this even needed?
        if (!field_try_draw_tetromino(&ctx.field, ctx.pos, ctx.tet) &&
            field_try_draw_tetromino(&ctx.field, VEC2_ADD(ctx.pos, POS(0, 1)), ctx.tet)) {
            ctx.pos.y++;
        }

        if (!field_try_draw_tetromino(&ctx.field, ctx.pos, ctx.tet)) {
            ctx.game_over = true;
            break;
        }

	ctx.lock_penalty = 0;
        while (field_try_draw_tetromino(&ctx.field, VEC2_SUB(ctx.pos, POS(0, 1)), ctx.tet)) {
        airborne:
            update_ghost_piece(&ctx);
            field_draw_tetromino(&ctx.field, ctx.ghost_pos, &ctx.ghost);
            field_draw_tetromino(&ctx.field, ctx.pos, ctx.tet);
            update_field(&ctx.field);
            field_clear_tetromino(&ctx.field, ctx.pos, ctx.tet);
            field_clear_tetromino(&ctx.field, ctx.ghost_pos, &ctx.ghost);
            uint32_t wait_until = timer_get_ms() + FALL_TIME(ctx.level);
            while (wait_until > timer_get_ms()) {
                if (!input_is_available(CONTROLLER_1)) {
                    continue;
                }

                // While we wait for the piece to fall down, we accept
                // controls and process the resulting instructions
                // from the user.  `process_controls` already changes
                // the state of ctx appropriate to the inputed
                // controls, but it can't control the game loop.
                instructions inst = process_controls(&ctx);

                // Happens when the user puts a piece to hold, we just
                // need to respawn as the switching was already taken
                // care of by `process_controls`.
                if (inst.respawn) {
                    render_hold_piece(&ctx.hold);
                    ctx.has_respawned = true;
                    goto respawn;
                }

                // If the input of the user resulted into changes on
                // the display, we need to display them.
                if (inst.rerender) {
                    update_ghost_piece(&ctx);
                    field_draw_tetromino(&ctx.field, ctx.ghost_pos, &ctx.ghost);
                    field_draw_tetromino(&ctx.field, ctx.pos, ctx.tet);
                    update_field(&ctx.field);
                    field_clear_tetromino(&ctx.field, ctx.pos, ctx.tet);
                    field_clear_tetromino(&ctx.field, ctx.ghost_pos, &ctx.ghost);
                }

                // If we are instructed to place a piece while still
                // airborne, it basically means we need to hard-drop
                // the piece.
                if (inst.place) {
                    // We don't want to lock the piece, thats why we
                    // set a lock penalty higher than the lock
                    // time. Making it skip the lock process.
                    ctx.lock_penalty = LOCK_TIME + 1;
                    break;
                }

                // When a piece needs to be locked it reached a ground
                // and is not airborne anymore.
                if (inst.lock) {
                    break;
                }

                // If we can't draw the piece below it self, we
                // reached ground and lock the piece.
                if (!field_try_draw_tetromino(&ctx.field, VEC2_SUB(ctx.pos, POS(0, 1)), ctx.tet)) {
                    break;
                }

                gpu_block_ack();
                gpu_block_frame();
            }
            ctx.pos.y--;
        }

        if (!field_try_draw_tetromino(&ctx.field, ctx.pos, ctx.tet)) {
            ctx.pos.y++;
        }

        ctx.tet->style.color = color_get_light_var(ctx.tet->style.color);

        field_draw_tetromino(&ctx.field, ctx.pos, ctx.tet);
        update_field(&ctx.field);
        field_clear_tetromino(&ctx.field, ctx.pos, ctx.tet);

        uint32_t wait_until = timer_get_ms() + LOCK_TIME - ctx.lock_penalty;
        ctx.lock_penalty += 200;
        while (wait_until > timer_get_ms()) {
            instructions inst = process_controls(&ctx);

            if (inst.respawn) {
                render_hold_piece(&ctx.hold);
                ctx.has_respawned = true;
                goto respawn;
            }

            if (inst.rerender) {
                field_draw_tetromino(&ctx.field, ctx.pos, ctx.tet);
                update_field(&ctx.field);
                field_clear_tetromino(&ctx.field, ctx.pos, ctx.tet);
            }

            if (inst.lock || inst.place) {
                break;
            }

            if (field_try_draw_tetromino(&ctx.field, VEC2_SUB(ctx.pos, POS(0, 1)), ctx.tet)) {
                ctx.tet->style.color = color_get_normal_var(ctx.tet->style.color);
                goto airborne;
            }

            gpu_block_ack();
            gpu_block_frame();
        }

        ctx.tet->style.color = color_get_normal_var(ctx.tet->style.color);

        field_draw_tetromino(&ctx.field, ctx.pos, ctx.tet);

        uint8_t lines = field_clear_lines(&ctx.field);
        ctx.lines_cleared += lines;

        uint32_t reward = 0;

        switch (lines) {
        case 1:
            reward = SCORE_SINGLE(ctx.level);
            ctx.last_b2b = BROKEN;
            break;
        case 2:
            display_message("DOUBLE");
            reward = SCORE_DOUBLE(ctx.level);
            ctx.last_b2b = BROKEN;
            break;
        case 3:
            display_message("TRIPLE");
            reward = SCORE_TRIPLE(ctx.level);
            ctx.last_b2b = BROKEN;
            break;
        case 4:
            if (ctx.last_b2b == TETRIS) {
                display_message("B2B TETRIS");
                reward = SCORE_B2B_TETRIS(ctx.level);
            } else {
                display_message("TETRIS");
                reward = SCORE_TETRIS(ctx.level);
            }
            ctx.last_b2b = TETRIS;
            break;
        }

        add_score(&ctx.score, reward, 8);
        if (reward > SCORE_SINGLE(ctx.level)) {
            display_message("          ");
        }

        // check if we need to switch level.
        if (ctx.lines_cleared >= ctx.level * 10) {
            ctx.lines_cleared -= ctx.level * 10;
            next_level(&ctx);
        }
    }

    if (ctx.game_over) {
        gpu_print_text(FRONT_BUFFER, 50, 50, 1, 0, "GAME OVER");
        timer_block_ms(1000);
        goto tetris_quit;
    }

    while (true)
        ;

tetris_quit:
    destroy_game(&ctx);
    return CODE_EXIT;
}
