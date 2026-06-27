#include "app/motor_demo_app.h"

#include "bsp/board.h"
#include "drivers/chassis/mecanum.h"
#include "drivers/encoder/encoder.h"
#include "drivers/led/led.h"
#include "drivers/motor/motor.h"
#include "drivers/uart/uart.h"

#define COMMAND_BUFFER_SIZE 48U
#define CONTROL_TIMEOUT_LOOPS 20000UL

typedef struct {
    char buffer[COMMAND_BUFFER_SIZE];
    unsigned length;
    int (*read_char)(char *ch);
    void (*write_text)(const char *text);
} command_port_t;

static unsigned long g_control_timeout;
static int g_last_forward;
static int g_last_strafe;
static int g_last_rotate;

static command_port_t g_usb_control_port = {
    { 0 },
    0U,
    uart3_read_char_nonblocking,
    uart3_write,
};

static command_port_t g_download_control_port = {
    { 0 },
    0U,
    uart1_read_char_nonblocking,
    uart1_write,
};

static command_port_t g_bluetooth_port = {
    { 0 },
    0U,
    uart2_read_char_nonblocking,
    uart2_write,
};

static void delay_cycles(volatile unsigned cycles)
{
    while (cycles-- != 0U) {
    }
}

static int is_space(char ch)
{
    return ch == ' ' || ch == '\t';
}

static const char *skip_spaces(const char *text)
{
    while (is_space(*text)) {
        ++text;
    }
    return text;
}

static int parse_int(const char **text, int *value)
{
    int sign = 1;
    int result = 0;
    const char *cursor = skip_spaces(*text);

    if (*cursor == '-') {
        sign = -1;
        ++cursor;
    } else if (*cursor == '+') {
        ++cursor;
    }

    if (*cursor < '0' || *cursor > '9') {
        return 0;
    }

    while (*cursor >= '0' && *cursor <= '9') {
        result = (result * 10) + (*cursor - '0');
        ++cursor;
    }

    *value = result * sign;
    *text = cursor;
    return 1;
}

static void port_write_int(command_port_t *port, int value)
{
    char buffer[12];
    unsigned index = sizeof(buffer);
    unsigned magnitude;

    buffer[--index] = '\0';
    if (value < 0) {
        magnitude = (unsigned)(-value);
    } else {
        magnitude = (unsigned)value;
    }

    do {
        buffer[--index] = (char)('0' + (magnitude % 10U));
        magnitude /= 10U;
    } while (magnitude != 0U);

    if (value < 0) {
        buffer[--index] = '-';
    }

    port->write_text(&buffer[index]);
}

static void report_wheel_trims(command_port_t *port)
{
    unsigned wheel;

    port->write_text("TRIM");
    for (wheel = 0U; wheel < (unsigned)BOARD_MOTOR_COUNT; ++wheel) {
        port->write_text(" ");
        port_write_int(port, mecanum_get_wheel_trim(wheel));
    }
    port->write_text("\n");
}

static void report_encoder_counts(command_port_t *port)
{
    unsigned wheel;

    port->write_text("ENC");
    for (wheel = 0U; wheel < (unsigned)BOARD_MOTOR_COUNT; ++wheel) {
        port->write_text(" ");
        port_write_int(port, (int)encoder_get_count((board_motor_id_t)wheel));
    }
    port->write_text("\n");
}

static void report_control_state(command_port_t *port)
{
    unsigned wheel;

    port->write_text("PID ");
    port_write_int(port, mecanum_get_closed_loop());
    port->write_text(" DELTA");
    for (wheel = 0U; wheel < (unsigned)BOARD_MOTOR_COUNT; ++wheel) {
        port->write_text(" ");
        port_write_int(port, mecanum_get_wheel_delta(wheel));
    }
    port->write_text(" OUT");
    for (wheel = 0U; wheel < (unsigned)BOARD_MOTOR_COUNT; ++wheel) {
        port->write_text(" ");
        port_write_int(port, mecanum_get_wheel_output(wheel));
    }
    port->write_text("\n");
}

static void status_led_pulse(void)
{
    led1_on();
    delay_cycles(40000U);
    led1_off();
}

static void apply_velocity(int forward, int strafe, int rotate)
{
    g_last_forward = forward;
    g_last_strafe = strafe;
    g_last_rotate = rotate;
    g_control_timeout = CONTROL_TIMEOUT_LOOPS;
    mecanum_drive(forward, strafe, rotate);
}

static void stop_chassis(void)
{
    g_last_forward = 0;
    g_last_strafe = 0;
    g_last_rotate = 0;
    g_control_timeout = 0;
    mecanum_stop();
}

static void apply_single_motor(int wheel, int speed)
{
    g_last_forward = 0;
    g_last_strafe = 0;
    g_last_rotate = 0;
    g_control_timeout = CONTROL_TIMEOUT_LOOPS;
    motor_stop_all();
    motor_set((motor_id_t)(wheel - 1), speed);
}

static void process_command(command_port_t *port, const char *command)
{
    int forward;
    int strafe;
    int rotate;
    int wheel;
    int speed;
    int trim;
    int enabled;

    command = skip_spaces(command);
    if (*command == 'G' || *command == 'g') {
        report_wheel_trims(port);
        return;
    }

    if (*command == 'R' || *command == 'r') {
        report_encoder_counts(port);
        return;
    }

    if (*command == 'D' || *command == 'd') {
        report_control_state(port);
        return;
    }

    if (*command == 'P' || *command == 'p') {
        ++command;
        if (parse_int(&command, &enabled)) {
            mecanum_set_closed_loop(enabled);
            port->write_text("OK P ");
            port_write_int(port, mecanum_get_closed_loop());
            port->write_text("\n");
            return;
        }
    }

    if (*command == 'C' || *command == 'c') {
        encoder_reset_all();
        port->write_text("OK C\n");
        return;
    }

    if (*command == 'S' || *command == 's') {
        stop_chassis();
        return;
    }

    if (*command == 'V' || *command == 'v') {
        ++command;
        if (parse_int(&command, &forward) &&
            parse_int(&command, &strafe) &&
            parse_int(&command, &rotate)) {
            apply_velocity(forward, strafe, rotate);
            return;
        }
    }

    if (*command == 'T' || *command == 't') {
        ++command;
        if (parse_int(&command, &wheel) &&
            parse_int(&command, &trim) &&
            wheel >= 1 &&
            wheel <= (int)BOARD_MOTOR_COUNT &&
            mecanum_set_wheel_trim((unsigned)(wheel - 1), trim)) {
            port->write_text("OK T ");
            port_write_int(port, wheel);
            port->write_text(" ");
            port_write_int(port, trim);
            port->write_text("\n");
            return;
        }
    }

    if (*command == 'M' || *command == 'm') {
        ++command;
        if (parse_int(&command, &wheel) &&
            parse_int(&command, &speed) &&
            wheel >= 1 &&
            wheel <= (int)BOARD_MOTOR_COUNT) {
            apply_single_motor(wheel, speed);
            port->write_text("OK M ");
            port_write_int(port, wheel);
            port->write_text(" ");
            port_write_int(port, speed);
            port->write_text("\n");
            status_led_pulse();
            return;
        }
    }

    port->write_text("ERR\n");
}

static void poll_command_port(command_port_t *port)
{
    char ch;

    while (port->read_char(&ch)) {
        if (ch == '\r' || ch == '\n') {
            if (port->length > 0U) {
                port->buffer[port->length] = '\0';
                process_command(port, port->buffer);
                port->length = 0U;
            }
        } else if (port->length < (COMMAND_BUFFER_SIZE - 1U)) {
            port->buffer[port->length++] = ch;
        } else {
            port->length = 0U;
            port->write_text("ERR overflow\n");
        }
    }
}

static void poll_uart_commands(void)
{
    poll_command_port(&g_download_control_port);
    poll_command_port(&g_usb_control_port);
    poll_command_port(&g_bluetooth_port);
}

static void poll_safety(void)
{
    if (board_k2_is_pressed()) {
        stop_chassis();
        return;
    }

    if (g_control_timeout > 0UL) {
        --g_control_timeout;
        if (g_control_timeout == 0UL &&
            (g_last_forward != 0 || g_last_strafe != 0 || g_last_rotate != 0)) {
            stop_chassis();
            uart1_write("STOP timeout\n");
            uart3_write("STOP timeout\n");
            uart2_write("STOP timeout\n");
        }
    }
}

void motor_demo_app_init(void)
{
    board_init();
    board_k2_init();
    led_init();
    uart1_init(115200U);
    uart2_init(115200U);
    uart3_init(115200U);
    mecanum_init();
    encoder_init();
    mecanum_stop();

    uart1_write("\nROS Robot Controller download UART control ready\n");
    uart1_write("Commands: V, M, S, G, R, C, D, P <0|1>, T <wheel 1-4> <trim 500-1500>.\n");
    uart2_write("\nROS Robot Controller bluetooth UART ready\n");
    uart2_write("Commands: V, M, S, G, R, C, D, P <0|1>, T <wheel 1-4> <trim 500-1500>.\n");
    status_led_pulse();
}

void motor_demo_app_task(void)
{
    encoder_poll();
    mecanum_control_task();
    poll_uart_commands();
    poll_safety();
}
