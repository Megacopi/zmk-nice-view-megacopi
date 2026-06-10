#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/wpm.h>
#include <lvgl.h>
#include <string.h>

#define SCREEN_W        160
#define SCREEN_H        68
#define MAX_ROCKETS     6
#define MAX_PARTICLES   32
#define TICK_MS         50

// --- Rocket ---
typedef struct {
    bool active;
    int16_t x, y;
    int16_t vy;       // negative = moving up
    uint8_t trail;
} Rocket;

// --- Particle (explosion spark) ---
typedef struct {
    bool active;
    int16_t x, y;
    int16_t vx, vy;  // fixed-point *4
    uint8_t life;    // counts down to 0
} Particle;

static Rocket rockets[MAX_ROCKETS];
static Particle particles[MAX_PARTICLES];
static lv_obj_t *canvas;
static lv_color_t cbuf[SCREEN_W * SCREEN_H];

// Simple LCG pseudo-random (no stdlib rand needed)
static uint32_t rng_state = 12345;
static uint32_t my_rand(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

static void spawn_rocket(void) {
    // Find free slot
    for (int i = 0; i < MAX_ROCKETS; i++) {
        if (!rockets[i].active) {
            rockets[i].active = true;
            rockets[i].x = 8 + (my_rand() % (SCREEN_W - 16));
            rockets[i].y = SCREEN_H - 1;
            // Speed: idle=1..2, typing=3..5
            uint8_t wpm = zmk_wpm_get_state();
            int speed = (wpm > 20) ? (3 + my_rand() % 3) : (1 + my_rand() % 2);
            rockets[i].vy = -speed;
            rockets[i].trail = 0;
            return;
        }
    }
}

static void explode(int16_t x, int16_t y) {
    uint8_t wpm = zmk_wpm_get_state();
    int count = (wpm > 20) ? 20 : 10;
    int spawned = 0;
    for (int i = 0; i < MAX_PARTICLES && spawned < count; i++) {
        if (!particles[i].active) {
            particles[i].active = true;
            particles[i].x = x;
            particles[i].y = y;
            // Random direction, fixed-point *4
            int32_t vx = (int32_t)(my_rand() % 9) - 4;
            int32_t vy = (int32_t)(my_rand() % 9) - 4;
            particles[i].vx = (int16_t)vx;
            particles[i].vy = (int16_t)vy;
            particles[i].life = 8 + (my_rand() % 6);
            spawned++;
        }
    }
}

static void draw_frame(void) {
    // Clear canvas
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);

    lv_draw_rect_dsc_t dot;
    lv_draw_rect_dsc_init(&dot);
    dot.bg_color = lv_color_white();
    dot.bg_opa = LV_OPA_COVER;
    dot.radius = 0;

    // Draw rockets
    for (int i = 0; i < MAX_ROCKETS; i++) {
        if (!rockets[i].active) continue;
        if (rockets[i].x >= 0 && rockets[i].x < SCREEN_W &&
            rockets[i].y >= 0 && rockets[i].y < SCREEN_H) {
            lv_canvas_draw_rect(canvas, rockets[i].x, rockets[i].y, 2, 2, &dot);
        }
    }

    // Draw particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        int16_t px = particles[i].x;
        int16_t py = particles[i].y;
        if (px >= 0 && px < SCREEN_W && py >= 0 && py < SCREEN_H) {
            lv_canvas_draw_rect(canvas, px, py, 1, 1, &dot);
        }
    }
}

static void fireworks_timer_cb(lv_timer_t *timer) {
    uint8_t wpm = zmk_wpm_get_state();

    // Spawn logic: idle = ~every 4 ticks, typing = ~every 1-2 ticks
    uint32_t r = my_rand() % 8;
    bool should_spawn;
    if (wpm > 20) {
        should_spawn = (r < 5);  // ~60% chance per tick
    } else {
        should_spawn = (r < 2);  // ~25% chance per tick
    }

    // Count active rockets
    int active = 0;
    for (int i = 0; i < MAX_ROCKETS; i++) if (rockets[i].active) active++;

    int max_active = (wpm > 20) ? 5 : 2;
    if (should_spawn && active < max_active) {
        spawn_rocket();
    }

    // Update rockets
    for (int i = 0; i < MAX_ROCKETS; i++) {
        if (!rockets[i].active) continue;
        rockets[i].y += rockets[i].vy;

        // Explode near top (y < 10..20)
        int explode_y = 8 + (my_rand() % 12);
        if (rockets[i].y <= explode_y) {
            explode(rockets[i].x, rockets[i].y);
            rockets[i].active = false;
        }
    }

    // Update particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        particles[i].x += particles[i].vx / 2;
        particles[i].y += particles[i].vy / 2;
        particles[i].vy += 1;  // gravity
        particles[i].life--;
        if (particles[i].life == 0) {
            particles[i].active = false;
        }
    }

    draw_frame();
}

lv_obj_t *zmk_widget_fireworks_obj(void) {
    memset(rockets, 0, sizeof(rockets));
    memset(particles, 0, sizeof(particles));

    canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, cbuf, SCREEN_W, SCREEN_H, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
    lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_timer_create(fireworks_timer_cb, TICK_MS, NULL);
    return canvas;
}