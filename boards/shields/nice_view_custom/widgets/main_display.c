#include <zephyr/kernel.h>
#include <zmk/display.h>
#include <lvgl.h>

// Deklarationen
lv_obj_t *zmk_widget_layer_status_obj(void);
lv_obj_t *zmk_widget_battery_status_obj(void);
lv_obj_t *zmk_widget_fireworks_obj(void);

int zmk_display_init(void) {
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    // Linke Seite: Layer-Name + Batterie%
    zmk_widget_layer_status_obj();
    zmk_widget_battery_status_obj();
#else
    // Rechte Seite: Feuerwerk + Batterie%
    zmk_widget_fireworks_obj();
    zmk_widget_battery_status_obj();
#endif

    return 0;
}