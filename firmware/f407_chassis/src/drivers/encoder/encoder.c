#include "drivers/encoder/encoder.h"

#include "bsp/board.h"

typedef struct {
    board_gpio_pin_t phase_a;
    board_gpio_pin_t phase_b;
} encoder_channel_t;

static const encoder_channel_t g_encoder_channels[BOARD_MOTOR_COUNT] = {
    [BOARD_MOTOR_M1] = {
        { BOARD_GPIO_PORT_A, 0U, 0U },
        { BOARD_GPIO_PORT_A, 1U, 0U },
    },
    [BOARD_MOTOR_M2] = {
        { BOARD_GPIO_PORT_A, 15U, 0U },
        { BOARD_GPIO_PORT_B, 3U, 0U },
    },
    [BOARD_MOTOR_M3] = {
        { BOARD_GPIO_PORT_B, 6U, 0U },
        { BOARD_GPIO_PORT_B, 7U, 0U },
    },
    [BOARD_MOTOR_M4] = {
        { BOARD_GPIO_PORT_B, 4U, 0U },
        { BOARD_GPIO_PORT_B, 5U, 0U },
    },
};

static int32_t g_counts[BOARD_MOTOR_COUNT];
static uint8_t g_last_state[BOARD_MOTOR_COUNT];

static uint8_t read_state(const encoder_channel_t *channel)
{
    uint8_t a = (uint8_t)board_gpio_read_active(channel->phase_a);
    uint8_t b = (uint8_t)board_gpio_read_active(channel->phase_b);

    return (uint8_t)((a << 1U) | b);
}

static int transition_delta(uint8_t previous, uint8_t current)
{
    static const int8_t table[16] = {
         0,  1, -1,  0,
        -1,  0,  0,  1,
         1,  0,  0, -1,
         0, -1,  1,  0,
    };

    return table[((previous & 3U) << 2U) | (current & 3U)];
}

void encoder_init(void)
{
    unsigned i;

    board_init();
    for (i = 0U; i < (unsigned)BOARD_MOTOR_COUNT; ++i) {
        board_gpio_input_pullup_init(g_encoder_channels[i].phase_a);
        board_gpio_input_pullup_init(g_encoder_channels[i].phase_b);
        g_last_state[i] = read_state(&g_encoder_channels[i]);
        g_counts[i] = 0;
    }
}

void encoder_poll(void)
{
    unsigned i;

    for (i = 0U; i < (unsigned)BOARD_MOTOR_COUNT; ++i) {
        uint8_t current = read_state(&g_encoder_channels[i]);
        int delta = transition_delta(g_last_state[i], current);

        g_counts[i] += delta;
        g_last_state[i] = current;
    }
}

void encoder_reset_all(void)
{
    unsigned i;

    for (i = 0U; i < (unsigned)BOARD_MOTOR_COUNT; ++i) {
        g_last_state[i] = read_state(&g_encoder_channels[i]);
        g_counts[i] = 0;
    }
}

int32_t encoder_get_count(board_motor_id_t motor)
{
    if ((unsigned)motor >= (unsigned)BOARD_MOTOR_COUNT) {
        return 0;
    }

    return g_counts[(unsigned)motor];
}
