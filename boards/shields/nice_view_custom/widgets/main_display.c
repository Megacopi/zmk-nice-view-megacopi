#include <zephyr/kernel.h>
#include <zmk/display.h>
#include <lvgl.h>

lv_obj_t *zmk_widget_layer_status_obj(void);
lv_obj_t *zmk_widget_battery_status_obj(void);
lv_obj_t *zmk_widget_fireworks_obj(void);

int zmk_display_status_screen_init(void) {
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    zmk_widget_layer_status_obj();
    zmk_widget_battery_status_obj();
#else
    zmk_widget_fireworks_obj();
    zmk_widget_battery_status_obj();
#endif

    return 0;
}