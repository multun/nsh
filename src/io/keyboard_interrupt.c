#include <nsh_io/keyboard_interrupt.h>

struct exception_type g_keyboard_interrupt = {
    .compat_error = NSH_KEYBOARD_INTERUPT,
};
