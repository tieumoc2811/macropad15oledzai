/*
 * &lcycle - chuyen vong layer moi lan bam.
 * Tuong ung keycode LCYCLE trong firmware QMK/Vial cua Macropad15:
 * nut nhan encoder lon (ENC1) chuyen Base -> Numpad -> Media -> Nokia -> Base ...
 * Danh sach layer lay tu property "layers" (layer index) trong devicetree.
 */
#define DT_DRV_COMPAT zmk_behavior_lcycle

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/keymap.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_lcycle_config {
    size_t layers_len;
    const uint32_t *layers;
};

struct behavior_lcycle_data {
    uint8_t idx;
};

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_lcycle_config *cfg = dev->config;
    struct behavior_lcycle_data *data = dev->data;

    if (cfg->layers == NULL || cfg->layers_len == 0) {
        return ZMK_BEHAVIOR_OPAQUE;
    }

    data->idx = (data->idx + 1) % cfg->layers_len;
    zmk_keymap_layer_id_t layer_id =
        zmk_keymap_layer_index_to_id((zmk_keymap_layer_index_t)cfg->layers[data->idx]);

    LOG_DBG("layer cycle -> index %d (id %d)", cfg->layers[data->idx], layer_id);
    /* Tuong ung TO() cua QMK: dat layer mac dinh moi */
    zmk_keymap_layer_to(layer_id, false);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_lcycle_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define LCYCLE_INST(n)                                                                             \
    BUILD_ASSERT(DT_INST_PROP_LEN(n, layers) > 0, "lcycle: can it nhat 1 layer trong layers");      \
    static const uint32_t behavior_lcycle_layers_##n[] = DT_INST_PROP(n, layers);                   \
    static const struct behavior_lcycle_config behavior_lcycle_config_##n = {                       \
        .layers_len = DT_INST_PROP_LEN(n, layers),                                                  \
        .layers = behavior_lcycle_layers_##n,                                                       \
    };                                                                                             \
    static struct behavior_lcycle_data behavior_lcycle_data_##n = {0};                              \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, &behavior_lcycle_data_##n, &behavior_lcycle_config_##n,  \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                       \
                            &behavior_lcycle_driver_api);

DT_INST_FOREACH_STATUS_OKAY(LCYCLE_INST)
