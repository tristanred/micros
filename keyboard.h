#ifndef KEYBOARD_H
#define KEYBOARD_H


#include <stddef.h>
#include <stdint.h>

#include "idt.h"
#include "common.h"
#include "vector.h"

/*
 * OS Keycodes list. This list is in the same order as the scancode set 1 so
 * mapping is 1:1.
 */
enum keycodes
{
    A = 0,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    ZERO = 26,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGTH,
    NINE,
    BACKTICK,
    MINUS,
    EQUAL,
    BACKSLASH,
    BACKSPACE,
    SPACE,
    TAB,
    CAPS,
    LEFT_SHIFT,
    LEFT_CTRL,
    LEFT_GUI, // Windows key ?
    LEFT_ALT,
    RIGHT_SHIFT,
    RIGHT_CTRL,
    RIGTH_GUI,
    RIGHT_ALT,
    APPS, // Windows or function key ?
    ENTER,
    ESCAPE,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    PRNT_SCRN,
    SCROLL,
    PAUSE,
    LEFT_BRACKET,
    INSERT,
    HOME,
    PG_UP,
    DELETE,
    END,
    PG_DN,
    UP_ARROW,
    LEFT_ARROW,
    DOWN_ARROW,
    RIGHT_ARROW,
    NUM_LOCK,
    KEYPAD_DIV,
    KEYPAD_MUL,
    KEYPAD_MINUS,
    KEYPAD_PLUS,
    KEYPAD_ENTER,
    KEYPAD_DOT,
    KEYPAD_0,
    KEYPAD_1,
    KEYPAD_2,
    KEYPAD_3,
    KEYPAD_4,
    KEYPAD_5,
    KEYPAD_6,
    KEYPAD_7,
    KEYPAD_8,
    KEYPAD_9,
    RIGHT_BRACKET,
    SEMI_COLON,
    APOSTROPHE,
    COMMA,
    DOT,
    FRONT_SLASH
};

enum scancode_sets
{
    SCANCODE_SET1,
    SCANCODE_SET2,
    SCANCODE_SET3,
};

size_t scancode_sets_keys;
uint16_t scancode_set1_values[104];
uint16_t scancode_set2_values[104];
uint16_t scancode_set3_values[104];

uint16_t* current_scancode_set;

// ascii map without any key modifiers
uint8_t keycode_ascii_map[104];

// State of the keys, 0 for released and 1 for pressed
uint8_t key_states[104];

enum
{
    KEYDOWN = 0,
    KEYUP = 1
};

typedef uint16_t keycode;

struct keyboard_state
{
    uint8_t currentScancode;
    uint16_t currentKeycode;
    
    uint8_t* keyStates;
    
    /*
     * | 7 6 5 4 |  |   3    |   2    |   1   |   0   |
     *   Unused        NUM     CapsLk   Shift   CTRL
     */
    uint8_t flags;
} current_keyboard_state;
typedef struct keyboard_state keyboard_state_t;

typedef struct
{
    int key_state;
    
    keycode key;
    
} keyevent_info;

typedef void (*hookfn)(keyevent_info*);
vector* keyboard_hooks;


// Driver internal methods
void keyboard_interrupt_handler(registers_t regs);

void SetFlagsFromKey(keyboard_state_t* state, BOOL keyPressed);

// Public methods
void GetKeyboardState(keyboard_state_t* kb);
unsigned char GetAscii(keycode k);
BOOL IsKeyDown(keycode k);
int IsPrintableCharacter(keycode k);
BOOL IsControlCharacter(keycode code);

void RegisterKeyboardHook(hookfn function);
void DeregisterKeyboardHook(hookfn function);

// Kernel methods
void SetupKeyboardDriver(int keyboard_scancodes);
 
void InitializeScancodeSetOne();

#endif
