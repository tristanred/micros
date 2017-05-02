#include "keyboard.h"
#include "framebuffer.h"
#include "io_func.h"
#include "vector.h"
#include "kernel_log.h"

void keyboard_interrupt_handler(registers_t regs)
{
    (void)regs;
    
    // Reading from keyboard IO port 0x60
    // Result is scancode of the key
    // qemu keyboard seems to be scancodes set 1 but set 2 is apparently the
    // more popular.
    // See : http://www.osdever.net/papers/view/ibm-pc-keyboard-information-for-software-developers
    unsigned char res = inb(0x60);

    current_keyboard_state.currentScancode = res;
    
    // TODO : To avoid doing a table scan each time, the keycodes and scancode
    // should be ordered by value in the list so scancode == keycode index
    for(size_t i = 0; i < scancode_sets_keys; i++)
    {
        if(current_scancode_set[i] == res)
        {
            current_keyboard_state.currentKeycode = i;
        }
    }
    
    BOOL keyDown = !(res >= 0x80);
    if(!keyDown)
    {
        kWriteLog_format1d("KEY RELEASE %d", (uint64_t)current_keyboard_state.currentKeycode);
        //current_keyboard_state.inputType = KEY_RELEASE;
        key_states[current_keyboard_state.currentKeycode] = 0;
    }
    else
    {
        kWriteLog_format1d("KEY PRESS %d", (uint64_t)current_keyboard_state.currentKeycode);
        //current_keyboard_state.inputType = KEY_DOWN;
        key_states[current_keyboard_state.currentKeycode] = 1;
    }
    
    if(IsControlCharacter(current_keyboard_state.currentKeycode))
    {
        SetFlagsFromKey(&current_keyboard_state, keyDown);
    }
    
    for(size_t i = 0; i < keyboard_hooks->count; i++)
    {
        hookfn func = (hookfn)keyboard_hooks->dataElements[i];
        
        keyevent_info info;
        
        // TODO : Simplify ?
        info.key_state = keyDown ? KEYDOWN : KEYUP;
        
        info.key = current_keyboard_state.currentKeycode;
        
        func(&info);
    }
}

BOOL IsControlCharacter(keycode code)
{
    if( (code >= CAPS && code <= RIGHT_CTRL) )
        return TRUE;
        
    return FALSE;
}

BOOL IsKeyDown(keycode k)
{
    return key_states[k] == 1;
}

void SetFlagsFromKey(keyboard_state_t* state, BOOL keyPressed)
{
   if(state->currentKeycode == LEFT_SHIFT || state->currentKeycode == RIGHT_SHIFT)
   {
       if(keyPressed)
       {
           state->flags |= 0x2;
       }
       else
       {
           state->flags &= ~(1 << 1);
       }
   }

   if(state->currentKeycode == LEFT_CTRL || state->currentKeycode == RIGHT_CTRL)
   {
       if(keyPressed)
       {
           state->flags |= 0x1;
       }
       else
       {
           state->flags &= ~(1 << 0);
       }
   }

   if(state->currentKeycode == CAPS)
   {
       if(keyPressed)
       {
           state->flags |= 0x4;
       }
       else
       {
           state->flags &= ~(1 << 2);
       }
   }

   if(state->currentKeycode == NUM_LOCK)
   {
       if(keyPressed)
       {
           state->flags |= 0x8;
       }
       else
       {
           state->flags &= ~(1 << 3);
       }
   }
}

void GetKeyboardState(keyboard_state_t* kb)
{
    if(kb != 0)
    {
        kb->currentScancode = current_keyboard_state.currentScancode;
        kb->currentKeycode = current_keyboard_state.currentKeycode;
    }
}

unsigned char GetAscii(keycode k)
{
    return keycode_ascii_map[k];
}

int IsPrintableCharacter(keycode k)
{
    if(k <= 36)
    {
        return 1;
    }
    
    return 0;
}

void RegisterKeyboardHook(hookfn function)
{
    vector_add(keyboard_hooks, function);
}

void DeregisterKeyboardHook(hookfn function)
{
    vector_remove(keyboard_hooks, function);
}

void SetupKeyboardDriver(int keyboard_scancodes)
{
    scancode_sets_keys = 104;
    
    current_keyboard_state.keyStates = key_states;
    
    keyboard_hooks = vector_create();
    
    switch(keyboard_scancodes)
    {
        case SCANCODE_SET1:
        {
            InitializeScancodeSetOne();
            break;
        }
        case SCANCODE_SET2:
        case SCANCODE_SET3:
        default:
        {
            // Set not supported
            break;
        }
    }
}

void InitializeScancodeSetOne()
{
    scancode_set1_values[0] = 0x1e;
    scancode_set1_values[1] = 0x30;
    scancode_set1_values[2] = 0x2e;
    scancode_set1_values[3] = 0x20;
    scancode_set1_values[4] = 0x12;
    scancode_set1_values[5] = 0x21;
    scancode_set1_values[6] = 0x22;
    scancode_set1_values[7] = 0x23;
    scancode_set1_values[8] = 0x17;
    scancode_set1_values[9] = 0x24;
    scancode_set1_values[10] = 0x25;
    scancode_set1_values[11] = 0x26;
    scancode_set1_values[12] = 0x32;
    scancode_set1_values[13] = 0x31;
    scancode_set1_values[14] = 0x18;
    scancode_set1_values[15] = 0x19;
    scancode_set1_values[16] = 0x10;
    scancode_set1_values[17] = 0x13;
    scancode_set1_values[18] = 0x1f;
    scancode_set1_values[19] = 0x14;
    scancode_set1_values[20] = 0x16;
    scancode_set1_values[21] = 0x2f;
    scancode_set1_values[22] = 0x11;
    scancode_set1_values[23] = 0x2d;
    scancode_set1_values[24] = 0x15;
    scancode_set1_values[25] = 0x2c;
    scancode_set1_values[26] = 0x0b;
    scancode_set1_values[27] = 0x02;
    scancode_set1_values[28] = 0x03;
    scancode_set1_values[29] = 0x04;
    scancode_set1_values[30] = 0x05;
    scancode_set1_values[31] = 0x06;
    scancode_set1_values[32] = 0x07;
    scancode_set1_values[33] = 0x08;
    scancode_set1_values[34] = 0x09;
    
    scancode_set1_values[35] = 0x0a;
    scancode_set1_values[36] = 0x29;
    scancode_set1_values[37] = 0x0c;
    scancode_set1_values[38] = 0x0d;
    scancode_set1_values[39] = 0x2b;
    scancode_set1_values[40] = 0x0e;
    scancode_set1_values[41] = 0x39;
    scancode_set1_values[42] = 0x0f;
    scancode_set1_values[43] = 0x3a;
    scancode_set1_values[44] = 0x2a;
    scancode_set1_values[45] = 0x1d;
    scancode_set1_values[46] = (0xE0 << 8) + 0x5b; // E0 5B
    scancode_set1_values[47] = 0x38;
    scancode_set1_values[48] = 0x36;
    scancode_set1_values[49] = (0xE0 << 8) + 0x1d; // E0 1D
    scancode_set1_values[50] = (0xE0 << 8) + 0x5c; // E0 5C
    scancode_set1_values[51] = (0xE0 << 8) + 0x38;
    scancode_set1_values[52] = (0xE0 << 8) + 0x5d;
    scancode_set1_values[53] = 0x1c;
    scancode_set1_values[54] = 0x01;
    scancode_set1_values[55] = 0x3b;
    scancode_set1_values[56] = 0x3c;
    scancode_set1_values[57] = 0x3d;
    scancode_set1_values[58] = 0x3e;
    scancode_set1_values[59] = 0x3f;
    scancode_set1_values[60] = 0x40;
    scancode_set1_values[61] = 0x41;
    scancode_set1_values[62] = 0x42;
    scancode_set1_values[63] = 0x43;
    scancode_set1_values[64] = 0x44;
    scancode_set1_values[65] = 0x57;
    scancode_set1_values[66] = 0x58;
    scancode_set1_values[67] = (0xE0 << 8) + 0x2a; // and 0xe0 0x37
    scancode_set1_values[68] = 0x46;
    scancode_set1_values[69] = (0xE1 << 8) + 0x1d; // Pause, bunch of stuff
    
    scancode_set1_values[70] = 0x1a;
    scancode_set1_values[71] = (0xE0 << 8) + 0x52;
    scancode_set1_values[72] = (0xE0 << 8) + 0x47;
    scancode_set1_values[73] = (0xE0 << 8) + 0x49;
    scancode_set1_values[74] = (0xE0 << 8) + 0x53;
    scancode_set1_values[75] = (0xE0 << 8) + 0x4f;
    scancode_set1_values[76] = (0xE0 << 8) + 0x51;
    scancode_set1_values[77] = (0xE0 << 8) + 0x48;
    scancode_set1_values[78] = (0xE0 << 8) + 0x4b;
    scancode_set1_values[79] = (0xE0 << 8) + 0x50;
    scancode_set1_values[80] = (0xE0 << 8) + 0x4d;
    scancode_set1_values[81] = 0x45;
    scancode_set1_values[82] = (0xE0 << 8) + 0x35;
    scancode_set1_values[83] = 0x37;
    scancode_set1_values[84] = 0x4a;
    scancode_set1_values[85] = 0x4e;
    scancode_set1_values[86] = (0xE0 << 8) + 0x1c;
    scancode_set1_values[87] = 0x53;
    scancode_set1_values[88] = 0x52;
    scancode_set1_values[89] = 0x4f;
    scancode_set1_values[90] = 0x50;
    scancode_set1_values[91] = 0x51;
    scancode_set1_values[92] = 0x4b;
    scancode_set1_values[93] = 0x4c;
    scancode_set1_values[94] = 0x4d;
    scancode_set1_values[95] = 0x47;
    scancode_set1_values[96] = 0x48;
    scancode_set1_values[97] = 0x49;
    scancode_set1_values[98] = 0x1b;
    scancode_set1_values[99] = 0x27;
    scancode_set1_values[100] = 0x28;
    scancode_set1_values[101] = 0x33;
    scancode_set1_values[102] = 0x34;
    scancode_set1_values[103] = 0x35;
    
    current_scancode_set = scancode_set1_values;
    
    keycode_ascii_map[0] = 0x61;
    keycode_ascii_map[1] = 0x62;
    keycode_ascii_map[2] = 0x63;
    keycode_ascii_map[3] = 0x64;
    keycode_ascii_map[4] = 0x65;
    keycode_ascii_map[5] = 0x66;
    keycode_ascii_map[6] = 0x67;
    keycode_ascii_map[7] = 0x68;
    keycode_ascii_map[8] = 0x69;
    keycode_ascii_map[9] = 0x6a;
    keycode_ascii_map[10] = 0x6b;
    keycode_ascii_map[11] = 0x6c;
    keycode_ascii_map[12] = 0x6d;
    keycode_ascii_map[13] = 0x6e;
    keycode_ascii_map[14] = 0x6f;
    keycode_ascii_map[15] = 0x70;
    keycode_ascii_map[16] = 0x71;
    keycode_ascii_map[17] = 0x72;
    keycode_ascii_map[18] = 0x73;
    keycode_ascii_map[19] = 0x74;
    keycode_ascii_map[20] = 0x75;
    keycode_ascii_map[21] = 0x76;
    keycode_ascii_map[22] = 0x77;
    keycode_ascii_map[23] = 0x78;
    keycode_ascii_map[24] = 0x79;
    keycode_ascii_map[25] = 0x7a;
    keycode_ascii_map[26] = 0x30;
    keycode_ascii_map[27] = 0x31;
    keycode_ascii_map[28] = 0x32;
    keycode_ascii_map[29] = 0x33;
    keycode_ascii_map[30] = 0x34;
    keycode_ascii_map[31] = 0x35;
    keycode_ascii_map[32] = 0x36;
    keycode_ascii_map[33] = 0x37;
    keycode_ascii_map[34] = 0x38;
    keycode_ascii_map[35] = 0x39;
    
    keycode_ascii_map[36] = 0x60;
    keycode_ascii_map[37] = 0x2d;
    keycode_ascii_map[38] = 0x3d;
    keycode_ascii_map[39] = 0x5c;
    keycode_ascii_map[40] = 0;
    keycode_ascii_map[41] = 0x20;
    keycode_ascii_map[42] = 0x09;
    keycode_ascii_map[43] = 0;
    keycode_ascii_map[44] = 0;
    keycode_ascii_map[45] = 0;
    keycode_ascii_map[46] = 0;
    keycode_ascii_map[47] = 0;
    keycode_ascii_map[48] = 0;
    keycode_ascii_map[49] = 0;
    keycode_ascii_map[50] = 0;
    keycode_ascii_map[51] = 0;
    keycode_ascii_map[52] = 0;
    keycode_ascii_map[53] = 0;
    keycode_ascii_map[54] = 0;
    keycode_ascii_map[55] = 0;
    keycode_ascii_map[56] = 0;
    keycode_ascii_map[57] = 0;
    keycode_ascii_map[58] = 0;
    keycode_ascii_map[59] = 0;
    keycode_ascii_map[60] = 0;
    keycode_ascii_map[61] = 0;
    keycode_ascii_map[62] = 0;
    keycode_ascii_map[63] = 0;
    keycode_ascii_map[64] = 0;
    keycode_ascii_map[65] = 0;
    keycode_ascii_map[66] = 0;
    keycode_ascii_map[67] = 0;
    keycode_ascii_map[68] = 0;
    keycode_ascii_map[69] = 0;
    
    keycode_ascii_map[70] = 0x5b;
    keycode_ascii_map[71] = 0;
    keycode_ascii_map[72] = 0;
    keycode_ascii_map[73] = 0;
    keycode_ascii_map[74] = 0;
    keycode_ascii_map[75] = 0;
    keycode_ascii_map[76] = 0;
    keycode_ascii_map[77] = 0;
    keycode_ascii_map[78] = 0;
    keycode_ascii_map[79] = 0;
    keycode_ascii_map[80] = 0;
    keycode_ascii_map[81] = 0;
    keycode_ascii_map[82] = 0x2f;
    keycode_ascii_map[83] = 0x2a;
    keycode_ascii_map[84] = 0x2d;
    keycode_ascii_map[85] = 0x2b;
    keycode_ascii_map[86] = 0;
    keycode_ascii_map[87] = 0x2e;
    keycode_ascii_map[88] = 0x30;
    keycode_ascii_map[89] = 0x31;
    keycode_ascii_map[90] = 0x32;
    keycode_ascii_map[91] = 0x33;
    keycode_ascii_map[92] = 0x34;
    keycode_ascii_map[93] = 0x35;
    keycode_ascii_map[94] = 0x36;
    keycode_ascii_map[95] = 0x37;
    keycode_ascii_map[96] = 0x38;
    keycode_ascii_map[97] = 0x39;
    keycode_ascii_map[98] = 0x5d;
    keycode_ascii_map[99] = 0x3b;
    keycode_ascii_map[100] = 0x27;
    keycode_ascii_map[101] = 0x2c;
    keycode_ascii_map[102] = 0x2e;
    keycode_ascii_map[103] = 0x2f;
}
