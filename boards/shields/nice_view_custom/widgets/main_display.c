#include <zephyr/kernel.h>
#include <zmk/display.h>
#include <lvgl.h>

lv_obj_t *zmk_widget_layer_status_obj(void);
lv_obj_t *zmk_widget_battery_status_obj(void);
lv_obj_t *zmk_widget_fireworks_obj(void);

int zmk_display_main(lv_obj_t *screen) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    zmk_widget_layer_status_obj();
    zmk_widget_battery_status_obj();
#else
    zmk_widget_fireworks_obj();
    zmk_widget_battery_status_obj();
#endif
    return 0;
}

ZMK_DISPLAY_INIT(zmk_display_main);