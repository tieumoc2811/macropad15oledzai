/*
 * &th_key <phim_bam> <phim_giu> - bam ngan = phim 1, bam giu = phim 2.
 * Tuong ung TH_MIN / TH_PLS trong firmware QMK/Vial cua Macropad15:
 *   &th_key KP_MINUS KP_DIVIDE  (bam "-", giu "/")
 *   &th_key KP_PLUS KP_MULTIPLY (bam "+", giu "*")
 * Thoi gian phan biet: CONFIG_MACROPAD15_TAPPING_TERM_MS (mac dinh 200ms, =
 * TAPPING_TERM trong QMK).
 *
 * Luu y giong ban QMK goc: cac phim tap-hold dung chung 1 bo theo doi nen
 * neu bam 2 phim tap-hold cung luc, phim bam sau se ghi de phim truoc.
 */
#define DT_DRV_COMPAT zmk_behavior_tap_hold_key

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/events/keycode_state_changed.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_tap_hold_config {
    uint32_t tapping_term_ms;
};

struct behavior_tap_hold_data {
    const struct device *dev;
    struct k_work_delayable timer_work;
    uint32_t hold_keycode;
    bool timer_fired;
    bool pressed;
};

static void tap_hold_timer_work_cb(struct k_work *work) {
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    struct behavior_tap_hold_data *data =
        CONTAINER_OF(dwork, struct behavior_tap_hold_data, timer_work);
    const struct behavior_tap_hold_config *cfg = data->dev->config;

    data->timer_fired = true;
    LOG_DBG("tap-hold: het thoi gian -> giu phim 0x%04X", data->hold_keycode);
    /* Vuot qua tapping term: bam giu phim thu hai */
    raise_zmk_keycode_state_changed_from_encoded(data->hold_keycode, true, k_uptime_get());
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_tap_hold_config *cfg = dev->config;
    struct behavior_tap_hold_data *data = dev->data;

    data->pressed = true;
    data->timer_fired = false;
    data->hold_keycode = binding->param2;

    k_work_reschedule(&data->timer_work, K_MSEC(cfg->tapping_term_ms));
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct behavior_tap_hold_data *data = dev->data;

    if (!data->pressed) {
        return ZMK_BEHAVIOR_OPAQUE;
    }
    data->pressed = false;

    if (data->timer_fired) {
        /* Dang giu phim thu hai: nha ra */
        raise_zmk_keycode_state_changed_from_encoded(data->hold_keycode, false, event.timestamp);
    } else {
        /* Bam ngan: nhan phim thu hai mot lan */
        k_work_cancel_delayable(&data->timer_work);
        LOG_DBG("tap-hold: bam nhanh -> tap phim 0x%04X", binding->param1);
        raise_zmk_keycode_state_changed_from_encoded(binding->param1, true, event.timestamp);
        raise_zmk_keycode_state_changed_from_encoded(binding->param1, false, event.timestamp);
    }
    data->timer_fired = false;
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_tap_hold_init(const struct device *dev) {
    struct behavior_tap_hold_data *data = dev->data;

    data->dev = dev;
    k_work_init_delayable(&data->timer_work, tap_hold_timer_work_cb);
    return 0;
}

static const struct behavior_driver_api behavior_tap_hold_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define TAP_HOLD_INST(n)                                                                           \
    static const struct behavior_tap_hold_config behavior_tap_hold_config_##n = {                   \
        .tapping_term_ms = DT_INST_PROP_OR(n, tapping_term_ms,                                      \
                                           CONFIG_MACROPAD15_TAPPING_TERM_MS),                      \
    };                                                                                             \
    static struct behavior_tap_hold_data behavior_tap_hold_data_##n = {0};                          \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_tap_hold_init, NULL, &behavior_tap_hold_data_##n,           \
                            &behavior_tap_hold_config_##n, POST_KERNEL,                             \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_tap_hold_driver_api);

DT_INST_FOREACH_STATUS_OKAY(TAP_HOLD_INST)
