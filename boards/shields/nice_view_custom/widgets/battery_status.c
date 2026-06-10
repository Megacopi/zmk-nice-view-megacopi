#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/battery.h>
#include <lvgl.h>

static lv_obj_t *battery_label;

static void update_battery(void) {
    uint8_t level = zmk_battery_state_of_charge();
    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", level);
    lv_label_set_text(battery_label, buf);
}

static int battery_event_handler(const zmk_event_t *eh) {
    update_battery();
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(battery_status, battery_event_handler);
ZMK_SUBSCRIPTION(battery_status, zmk_battery_state_changed);

lv_obj_t *zmk_widget_battery_status_obj(void) {
    battery_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_16, 0);
    lv_label_set_text(battery_label, "?%");
    lv_obj_align(battery_label, LV_ALIGN_BOTTOM_RIGHT, -4, -4);
    update_battery();
    return battery_label;
}