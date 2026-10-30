/*
 * Man hinh status custom cho Macropad15 (thay man hinh mac dinh cua ZMK).
 * Tuong ung OLED trong firmware QMK/Vial:
 *  - Khi khong bam phim: ten layer + trang thai NUM/CAPS + mod (SFT/CTL/ALT/GUI)
 *  - Khi bam phim: ten layer + PHIM VUA BAM bang chu to o giua man hinh,
 *    tu dong quay ve sau 700ms (SHOW_AFTER_MS cua QMK)
 *
 * Man hinh tat tu dong khi khong hoat dong nho
 * CONFIG_ZMK_DISPLAY_BLANK_ON_IDLE (theo OLED_TIMEOUT cua QMK).
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <lvgl.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/modifiers_state_changed.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/keymap.h>
#include <zmk/hid.h>
#include <zmk/hid_indicators.h>
#include <zmk/keys.h>

#include <dt-bindings/zmk/hid_usage_pages.h>
#include <dt-bindings/zmk/modifiers.h>

/* Giong SHOW_AFTER_MS trong keymap.c cua QMK */
#define SHOW_AFTER_MS 700

/* Bit trong bao cao HID LED (theo thu tu usage LED cua HID) */
#define HID_LED_NUM_LOCK 0x01
#define HID_LED_CAPS_LOCK 0x02

#define SHIFT_MASK (MOD_LSFT | MOD_RSFT)

static lv_obj_t *layer_label;
static lv_obj_t *key_label;
static lv_obj_t *status_label;
static bool show_key;

/* ---------- Doi ma phim thanh ten ngan (toi da ~5-6 ky tu, giong key_label()
 * trong QMK) ---------- */
static void append_key_name(const struct zmk_keycode_state_changed *ev, char *out, size_t len) {
    if (ev->usage_page == HID_USAGE_KEY) {
        uint32_t id = ev->keycode;

        /* Phim modifier (0xE0..0xE7) */
        if (id >= 0xE0 && id <= 0xE7) {
            static const char *mod_names[] = {"CTRL", "SHIFT", "ALT", "GUI"};
            strncat(out, mod_names[id - 0xE0], len - strlen(out) - 1);
            return;
        }

        /* Chu cai A..Z (0x04..0x1D) */
        if (id >= 0x04 && id <= 0x1D) {
            char c = 'A' + (id - 0x04);
            if (!(ev->implicit_modifiers & SHIFT_MASK)) {
                c = 'a' + (id - 0x04); /* viet thuong khi khong co Shift */
            }
            size_t l = strlen(out);
            if (l + 1 < len) {
                out[l] = c;
                out[l + 1] = '\0';
            }
            return;
        }

        /* Chu so 1..9, 0 (hang tren: 0x1E..0x27) va keypad 1..9, 0 (0x59..0x62) */
        if ((id >= 0x1E && id <= 0x27) || (id >= 0x59 && id <= 0x62)) {
            uint32_t n = (id < 0x40) ? (id - 0x1E + 1) % 10 : (id - 0x59 + 1) % 10;
            char digit = '0' + n;
            size_t l = strlen(out);
            if (l + 1 < len) {
                out[l] = digit;
                out[l + 1] = '\0';
            }
            return;
        }

        /* Phim chuc nang F1..F12 */
        if (id >= 0x3A && id <= 0x45) {
            char f[6];
            snprintf(f, sizeof(f), "F%d", (int)(id - 0x3A + 1));
            strncat(out, f, len - strlen(out) - 1);
            return;
        }

        switch (id) {
        case 0x28: /* RET/ENTER */
        case 0x58: /* KP_ENTER */
            strncat(out, "ENTER", len - strlen(out) - 1);
            return;
        case 0x29:
            strncat(out, "ESC", len - strlen(out) - 1);
            return;
        case 0x2A:
            strncat(out, "BKSP", len - strlen(out) - 1);
            return;
        case 0x2B:
            strncat(out, "TAB", len - strlen(out) - 1);
            return;
        case 0x2C:
            strncat(out, "SPACE", len - strlen(out) - 1);
            return;
        case 0x2D:
            strncat(out, "-", len - strlen(out) - 1);
            return;
        case 0x2E:
            strncat(out, "=", len - strlen(out) - 1);
            return;
        case 0x33:
            strncat(out, ";", len - strlen(out) - 1);
            return;
        case 0x34:
            strncat(out, "'", len - strlen(out) - 1);
            return;
        case 0x36:
            strncat(out, ",", len - strlen(out) - 1);
            return;
        case 0x37:
            strncat(out, ".", len - strlen(out) - 1);
            return;
        case 0x38:
            strncat(out, "/", len - strlen(out) - 1);
            return;
        case 0x4A:
            strncat(out, "HOME", len - strlen(out) - 1);
            return;
        case 0x4B:
            strncat(out, "PG UP", len - strlen(out) - 1);
            return;
        case 0x4C:
            strncat(out, "DEL", len - strlen(out) - 1);
            return;
        case 0x4D:
            strncat(out, "END", len - strlen(out) - 1);
            return;
        case 0x4E:
            strncat(out, "PG DN", len - strlen(out) - 1);
            return;
        case 0x4F:
            strncat(out, "RIGHT", len - strlen(out) - 1);
            return;
        case 0x50:
            strncat(out, "LEFT", len - strlen(out) - 1);
            return;
        case 0x51:
            strncat(out, "DOWN", len - strlen(out) - 1);
            return;
        case 0x52:
            strncat(out, "UP", len - strlen(out) - 1);
            return;
        case 0x53:
            strncat(out, "NUM", len - strlen(out) - 1);
            return;
        case 0x54: /* KP / */
        case 0x55: /* KP * */
        case 0x56: /* KP - */
        case 0x57: /* KP + */
        case 0x63: /* KP . */
        {
            char sym = (id == 0x54) ? '/' : (id == 0x55) ? '*' : (id == 0x56) ? '-' : (id == 0x57)
                                                                                   ? '+'
                                                                                   : '.';
            size_t l = strlen(out);
            if (l + 1 < len) {
                out[l] = sym;
                out[l + 1] = '\0';
            }
            return;
        }
        default:
            strncat(out, "KEY", len - strlen(out) - 1);
            return;
        }
    } else if (ev->usage_page == HID_USAGE_CONSUMER) {
        switch (ev->keycode) {
        case 0xB0: /* PLAY */
        case 0xB1: /* PAUSE */
        case 0xCD: /* PLAY/PAUSE */
            strncat(out, "PLAY", len - strlen(out) - 1);
            return;
        case 0xB3:
            strncat(out, "FF", len - strlen(out) - 1);
            return;
        case 0xB4:
            strncat(out, "RW", len - strlen(out) - 1);
            return;
        case 0xB5:
            strncat(out, "NEXT", len - strlen(out) - 1);
            return;
        case 0xB6:
            strncat(out, "PREV", len - strlen(out) - 1);
            return;
        case 0xE2:
            strncat(out, "MUTE", len - strlen(out) - 1);
            return;
        case 0xE9:
            strncat(out, "VOL +", len - strlen(out) - 1);
            return;
        case 0xEA:
            strncat(out, "VOL -", len - strlen(out) - 1);
            return;
        case 0x192: /* AL Calculator */
            strncat(out, "CALC", len - strlen(out) - 1);
            return;
        default:
            strncat(out, "MEDIA", len - strlen(out) - 1);
            return;
        }
    }
}

/* ---------- Che do hien thi: bam phim (chu to) / nghi (thong tin) ---------- */
static void refresh_status_text(void);

static void set_show_key(bool on) {
    if (show_key == on) {
        return;
    }
    show_key = on;
    if (on) {
        lv_obj_add_flag(status_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text(key_label, "");
        lv_obj_remove_flag(status_label, LV_OBJ_FLAG_HIDDEN);
        refresh_status_text();
    }
}

static void clear_key_work_cb(struct k_work *work) { set_show_key(false); }

static K_WORK_DELAYABLE_DEFINE(clear_key_work, clear_key_work_cb);

/* ---------- Listener: layer ---------- */
struct layer_state {
    char name[16];
};

static void layer_update_cb(struct layer_state state) {
    lv_label_set_text(layer_label, state.name);
}

static struct layer_state layer_get_state(const zmk_event_t *eh) {
    struct layer_state s = {0};
    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    zmk_keymap_layer_id_t id = zmk_keymap_layer_index_to_id(index);
    const char *name = zmk_keymap_layer_name(id);
    if (name != NULL && name[0] != '\0') {
        snprintf(s.name, sizeof(s.name), "Layer: %s", name);
    } else {
        snprintf(s.name, sizeof(s.name), "Layer: %d", index);
    }
    return s;
}

ZMK_DISPLAY_WIDGET_LISTENER(macropad_layer, struct layer_state, layer_update_cb, layer_get_state)
ZMK_SUBSCRIPTION(macropad_layer, zmk_layer_state_changed)

/* ---------- Listener: phim vua bam ---------- */
struct key_state {
    char text[12];
    bool valid;
};

static void key_update_cb(struct key_state state) {
    if (!state.valid) {
        return;
    }
    lv_label_set_text(key_label, state.text);
    set_show_key(true);
    k_work_reschedule_for_queue(zmk_display_work_q(), &clear_key_work, K_MSEC(SHOW_AFTER_MS));
}

/* Luu y: as_zmk_keycode_state_changed(NULL) se loi nen phai kiem tra eh truoc */
static struct key_state key_get_state(const zmk_event_t *eh) {
    struct key_state s = {.text = {0}, .valid = false};
    if (eh == NULL) {
        return s;
    }
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev != NULL && ev->state) {
        append_key_name(ev, s.text, sizeof(s.text));
        s.valid = s.text[0] != '\0';
    }
    return s;
}

ZMK_DISPLAY_WIDGET_LISTENER(macropad_key, struct key_state, key_update_cb, key_get_state)
ZMK_SUBSCRIPTION(macropad_key, zmk_keycode_state_changed)

/* ---------- Listener: mod + LED NUM/CAPS ---------- */
struct mod_state {
    uint8_t reserved;
};

static void refresh_status_text(void) {
    if (status_label == NULL) {
        return;
    }
    zmk_mod_flags_t mods = zmk_hid_get_explicit_mods();
    zmk_hid_indicators_t ind = zmk_hid_indicators_get_current_profile();

    char buf[48] = "";
    if (ind & HID_LED_NUM_LOCK) {
        strcat(buf, "NUM ");
    }
    if (ind & HID_LED_CAPS_LOCK) {
        strcat(buf, "CAPS ");
    }
    if (mods & MOD_LSFT) {
        strcat(buf, "SFT ");
    }
    if (mods & MOD_LCTL) {
        strcat(buf, "CTL ");
    }
    if (mods & MOD_LALT) {
        strcat(buf, "ALT ");
    }
    if (mods & MOD_LGUI) {
        strcat(buf, "GUI ");
    }
    if (buf[0] == '\0') {
        strcat(buf, "-");
    }
    lv_label_set_text(status_label, buf);
}

static void mod_update_cb(struct mod_state state) { refresh_status_text(); }

static struct mod_state mod_get_state(const zmk_event_t *eh) {
    struct mod_state s = {0};
    return s;
}

ZMK_DISPLAY_WIDGET_LISTENER(macropad_mod, struct mod_state, mod_update_cb, mod_get_state)
ZMK_SUBSCRIPTION(macropad_mod, zmk_modifiers_state_changed)
ZMK_SUBSCRIPTION(macropad_mod, zmk_hid_indicators_changed)

/* ---------- Man hinh ---------- */
lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_pad_all(screen, 0, 0);

    layer_label = lv_label_create(screen);
    lv_obj_set_style_text_font(layer_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(layer_label, LV_ALIGN_TOP_LEFT, 2, 0);
    lv_label_set_text(layer_label, "");

    key_label = lv_label_create(screen);
    lv_obj_set_style_text_font(key_label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_align(key_label, LV_ALIGN_TOP_MID, 0, 18);
    lv_label_set_text(key_label, "");

    status_label = lv_label_create(screen);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(status_label, LV_ALIGN_BOTTOM_LEFT, 2, 0);
    lv_label_set_text(status_label, "");

    show_key = false;

    macropad_layer_init();
    macropad_key_init();
    macropad_mod_init();

    return screen;
}
