#include "keyboard.h"
#include "framebuffer.h"
#include "io_func.h"

void keyboard_interrupt_handler(registers_t regs)
{
    // Reading from keyboard IO port 0x60
    // Result is scancode of the key
    // qemu keyboard seems to be scancodes set 1 but set 2 is apparently the
    // more popular.
    // See : http://www.osdever.net/papers/view/ibm-pc-keyboard-information-for-software-developers
    unsigned char res = inb(0x60);

    current_keyboard_state.currentScancode = res;
    
    fbPutChar('c');
}
