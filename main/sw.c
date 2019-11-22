#include <string.h>
#include "zephyr/types.h"
#include "util.h"
#include "sw.h"

enum {
    SW_A,
    SW_B,
    SW_Y,
    SW_X,
    SW_SL,
    SW_SR,
    SW_L,
    SW_ZL,
    SW_MINUS,
    SW_PLUS,
    SW_LJ,
    SW_RJ,
    SW_HOME,
    SW_CAPTURE,
    SW_R,
    SW_ZR,
};

const uint8_t sw_axes_idx[4] =
{
/*  AXIS_LX, AXIS_LY, AXIS_RX, AXIS_RY  */
    0,       2,       1,       3
};

const struct ctrl_meta sw_btn_meta =
{
    .polarity = 0,
};

const struct ctrl_meta sw_axes_meta =
{
    .neutral = 0x800,
    .abs_max = 0x44C,
};

struct sw_map {
    uint16_t buttons;
    uint8_t hat;
    uint16_t axes[4];
} __packed;

const uint32_t sw_mask[4] = {0xFFFF0FFF, 0x00000000, 0x00000000, 0x00000000};
const uint32_t sw_desc[4] = {0x000000FF, 0x00000000, 0x00000000, 0x00000000};

const uint32_t sw_btns_mask[32] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    BIT(SW_Y), BIT(SW_A), BIT(SW_B), BIT(SW_X),
    BIT(SW_PLUS), BIT(SW_MINUS), BIT(SW_HOME), BIT(SW_CAPTURE),
    BIT(SW_ZL), BIT(SW_L), BIT(SW_SL), BIT(SW_LJ),
    BIT(SW_ZR), BIT(SW_R), BIT(SW_SR), BIT(SW_RJ),
};

void sw_to_generic(struct bt_data *bt_data, struct generic_ctrl *ctrl_data) {
    struct sw_map *map = (struct sw_map *)bt_data->input;

    memset((void *)ctrl_data, 0, sizeof(*ctrl_data));

    ctrl_data->mask = (uint32_t *)sw_mask;
    ctrl_data->desc = (uint32_t *)sw_desc;

    for (uint32_t i = 0; i < ARRAY_SIZE(generic_btns_mask); i++) {
        if (map->buttons & sw_btns_mask[i]) {
            ctrl_data->btns[0].value |= generic_btns_mask[i];
        }
    }

    /* Convert hat to regular btns */
    ctrl_data->btns[0].value |= hat_to_ld_btns[map->hat & 0xF];

    if (!atomic_test_bit(&bt_data->flags, BT_INIT)) {
        for (uint32_t i = 0; i < ARRAY_SIZE(map->axes); i++) {
            bt_data->axes_cal[i] = -(map->axes[sw_axes_idx[i]] - sw_axes_meta.neutral);
        }
        atomic_set_bit(&bt_data->flags, BT_INIT);
    }

    for (uint32_t i = 0; i < ARRAY_SIZE(map->axes); i++) {
        ctrl_data->axes[i].meta = &sw_axes_meta;
        ctrl_data->axes[i].value = map->axes[sw_axes_idx[i]] - sw_axes_meta.neutral + bt_data->axes_cal[i];
    }

}