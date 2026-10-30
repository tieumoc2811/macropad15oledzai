/*
 * &nokia <so> - Nokia multitap: bam lien tiep cung phim trong cua so multitap
 * de doi ky tu (1abc, 2def, ..., 9yz, 0 + khoang cach, . , ? !).
 * Tuong ung NK_1..NK_DOT trong firmware QMK/Vial cua Macropad15.
 *
 *  param1 | ky tu
 *     1-9 | chu so tuong ung + chu cai (vd 7 = "7stu")
 *      0  | so 0 va khoang cach
 *     10  | . , ? !
 *
 * Bam lai cung phim trong CONFIG_MACROPAD15_NOKIA_MULTITAP_MS (600ms) se xoa
 * ky tu vua go va chuyen sang ky tu ke tiep (giong dien thoai Nokia).
 * Viet hoa khi Shift dang giu (ke ca &sk - sticky shift). Ghi chu: khac ban
 * QMK, trang thai Caps Word khong anh huong den multitap (ZMK khong co API
 * doc trang thai nay tu behavior khac).
 */
#define DT_DRV_COMPAT zmk_behavior_nokia_multitap

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>
#include <zmk/behavior.h>

#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/modifiers.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* Chi so 0..9 cho phim 1..0, chi so 10 cho phim . , ? ! */
static const char *const nokia_chars[] = {
    "1abc", "2def", "3ghi", "4jkl", "5mno", "6pqr", "7stu", "8vwx", "9yz", "0 ", ".,?!",
};
#define NOKIA_KEY_COUNT ARRAY_SIZE(nokia_chars)

#define SHIFT_MASK (MOD_LSFT | MOD_RSFT)

struct behavior_nokia_data {
    uint32_t last_key;
    int64_t last_time;
    uint8_t index;
    bool last_shift;
    bool had_last;
};

static int tap_char(char c, bool shift, int64_t timestamp) {
    uint32_t encoded;
    bool apply_shift = shift;

    if (c >= 'a' && c <= 'z') {
        encoded = A + (c - 'a');
    } else if (c >= '1' && c <= '9') {
        encoded = N1 + (c - '1');
        apply_shift = false; /* chu so khong viet hoa */
    } else if (c == '0') {
        encoded = N0;
        apply_shift = false;
    } else if (c == ' ') {
        encoded = SPACE;
        apply_shift = false;
    } else if (c == '.') {
        encoded = DOT;
        apply_shift = false;
    } else if (c == ',') {
        encoded = COMMA;
        apply_shift = false;
    } else if (c == '?') {
        encoded = LS(SLSH); /* Shift + / */
        apply_shift = false;
    } else if (c == '!') {
        encoded = LS(N1); /* Shift + 1 */
        apply_shift = false;
    } else {
        return -EINVAL;
    }

    if (apply_shift) {
        encoded = LS(encoded);
    }

    raise_zmk_keycode_state_changed_from_encoded(encoded, true, timestamp);
    return raise_zmk_keycode_state_changed_from_encoded(encoded, false, timestamp);
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct behavior_nokia_data *data = dev->data;
    uint32_t key_idx = binding->param1;

    if (key_idx >= NOKIA_KEY_COUNT) {
        LOG_WRN("nokia: phim %d khong hop le", key_idx);
        return ZMK_BEHAVIOR_OPAQUE;
    }

    const char *chars = nokia_chars[key_idx];
    size_t len = strlen(chars);

    bool repeat = data->had_last && data->last_key == key_idx &&
                  (event.timestamp - data->last_time) < CONFIG_MACROPAD15_NOKIA_MULTITAP_MS;

    if (repeat) {
        /* Vong lap ky tu: xoa ky tu vua go roi go ky tu ke tiep */
        data->index = (data->index + 1) % len;
        raise_zmk_keycode_state_changed_from_encoded(BSPC, true, event.timestamp);
        raise_zmk_keycode_state_changed_from_encoded(BSPC, false, event.timestamp);
    } else {
        data->index = 0;
        /* Viet hoa neu Shift (hoac sticky shift) dang giu - giong get_mods()
         * | get_oneshot_mods() trong QMK */
        data->last_shift = (zmk_hid_get_explicit_mods() & SHIFT_MASK) != 0;
    }

    tap_char(chars[data->index], data->last_shift, event.timestamp);

    data->last_key = key_idx;
    data->last_time = event.timestamp;
    data->had_last = true;
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_nokia_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define NOKIA_INST(n)                                                                              \
    static struct behavior_nokia_data behavior_nokia_data_##n = {0};                                \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, &behavior_nokia_data_##n, NULL, POST_KERNEL,             \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_nokia_driver_api);

DT_INST_FOREACH_STATUS_OKAY(NOKIA_INST)
