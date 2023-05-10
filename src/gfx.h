/*
 * This is free and unencumbered software released into the public domain.
 *
 * This file is a tailored version of the mesgraphics.h file provided
 * with the MES source code, made to suit the needs for this game.
 *
 * Please feel free to use it.
 */

#ifndef GFX_H
#define GFX_H

#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>

#define BUFFER_BPP 3

#define BUFFER_SIZE(SIZE)                                                      \
    (((uint16_t)ceil((float)(BUFFER_BPP * (((SIZE).x) * ((SIZE).y))) / 8.0) %  \
      BUFFER_BPP) *                                                            \
         3 +                                                                   \
     ((BUFFER_BPP * (((SIZE).x) * ((SIZE).y))) / 8))

#define BUFFER_POSITION(RECT, COORD)                                           \
    (((COORD).y) * (RECT)->size.x + ((COORD).x))

#define PIXEL_MASK ((1 << BUFFER_BPP) - 1)

#define POS(X, Y)                                                              \
    (coord_t) { .x = X, .y = Y }

#define VEC2_ADD(LHS, RHS)                                                     \
    (typeof(LHS)) { .x = (LHS).x + (RHS).x, .y = (LHS).y + (RHS).y, }

#define VEC2_SUB(LHS, RHS)                                                     \
    (typeof(LHS)) { .x = (LHS).x - (RHS).x, .y = (LHS).y - (RHS).y, }

#define VEC2_MUL(LHS, RHS)                                                     \
    (typeof(LHS)) { .x = (LHS).x * (RHS).x, .y = (LHS).y * (RHS).y, }

#define VEC2_DIV(LHS, RHS)                                                     \
    (typeof(LHS)) { .x = (LHS).x / (RHS).x, .y = (LHS).y / (RHS).y, }

#define VEC2_SCALE(VEC, SCALE)                                                 \
    (typeof(VEC)) { .x = (VEC).x * SCALE, .y = (VEC).y * SCALE, }

#define SWAP(A, B)                                                             \
    {                                                                          \
        typeof(*(A)) _swap_temp = *(A);                                        \
        *(A) = *(B);                                                           \
        *(B) = _swap_temp;                                                     \
    }

#define BIT(X) (1 << (X))

/// Only the first 3 bytes are used.
typedef uint8_t pixel_t;

typedef struct {
    uint8_t x;
    uint8_t y;
} coord_t, screen_size_t;

typedef struct {
    screen_size_t size;
    void *pixels;
} buffer_t;

static buffer_t buffer_alloc(screen_size_t size) {
    return (buffer_t){size, malloc(BUFFER_SIZE(size))};
}

static void buffer_destory(buffer_t *buf) {
    free(buf->pixels);
    //free(buf);
}

static void buffer_set_pixel(buffer_t *buf, coord_t pos, pixel_t pixel) {
    uint16_t index = BUFFER_POSITION(buf, pos);
    uint32_t *pixels = (uint32_t *)(buf->pixels + (index / 8) * BUFFER_BPP);
    uint8_t shift = (7 - (index % 8)) * BUFFER_BPP;
    uint32_t mask = PIXEL_MASK << shift;
    *pixels = (*pixels & ~mask) | ((pixel & PIXEL_MASK) << shift);
}

static pixel_t buffer_get_pixel(const buffer_t *buf, coord_t pos) {
    uint16_t index = BUFFER_POSITION(buf, pos);
    uint32_t pixels = (*(uint32_t *)(buf->pixels + (index / 8) * BUFFER_BPP));
    return (pixels >> ((7 - (index % 8)) * BUFFER_BPP)) & PIXEL_MASK;
}

static void buffer_draw_line(buffer_t *rect, uint8_t x0, uint8_t y0,
                             uint8_t x1, uint8_t y1, uint8_t color) {
    bool steep = false;
    if (abs(x0 - x1) < abs(y0 - y1)) {
        SWAP(&x0, &y0);
        SWAP(&x1, &y1);
        steep = true;
    }
    if (x0 > x1) {
        SWAP(&x0, &x1);
        SWAP(&y0, &y1);
    }
    int16_t dx = x1 - x0;
    int16_t dy = y1 - y0;
    int16_t derror2 = abs(dy) * 2;
    int16_t error2 = 0;
    uint8_t y = y0;
    for (uint8_t x = x0; x < x1; ++x) {
        if (steep) {
            buffer_set_pixel(rect, POS(y, x), color);
        } else {
            buffer_set_pixel(rect, POS(x, y), color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

#endif // GFX_H
