#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>
#include <lvgl.h>

static lv_obj_t *layer_label;

static void update_layer_display(void) {
    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    const char *name = zmk_keymap_layer_name(index);
    if (name == NULL || name[0] == '\0') {
        char buf[8];
        snprintf(buf, sizeof(buf), "L%d", index);
        lv_label_set_text(layer_label, buf);
    } else {
        lv_label_set_text(layer_label, name);
    }
}

static int layer_event_handler(const zmk_event_t *eh) {
    update_layer_display();
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(layer_status, layer_event_handler);
ZMK_SUBSCRIPTION(layer_status, zmk_layer_state_changed);

lv_obj_t *zmk_widget_layer_status_obj(void) {
    layer_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(layer_label, &lv_font_montserrat_26, 0);
    lv_label_set_text(layer_label, "L0");
    lv_obj_align(layer_label, LV_ALIGN_CENTER, 0, 0);
    update_layer_display();
    return layer_label;
}

#endif /* IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) */