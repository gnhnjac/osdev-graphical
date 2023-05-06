#include <stdbool.h>

#define INPUT_BUFFER_SIZE 200

#define LSHIFT_PRESS 42
#define LSHIFT_REL 170

#define RSHIFT_PRESS 54
#define RSHIFT_REL 182

#define CTRL_PRESS 29
#define CTRL_REL 157

#define ALT_PRESS 56
#define ALT_REL 184

#define CAPS_PRESS 58
#define CAPS_REL 186

#define PS_DATA 0x60
#define PS_CTRL 0x64

#define DOWN_ARROW_PRESS 80
#define UP_ARROW_PRESS 72

#define WRITE_TO_KEYBOARD 0xD2

//refs
void enable_shift();
void disable_shift();
int check_shift();
void enable_ctrl();
void disable_ctrl();
int check_ctrl();
void enable_caps();
void disable_caps();
int check_caps();
void enable_alt();
void disable_alt();
int check_alt();
char kybrd_ctrl_read_status ();
void keyboard_handler(struct regs *r);
static void handle_character(unsigned char scancode);
void virtual_keyboard_input(unsigned char ascii);
void keyboard_input(int row, int col, char *buffer, int bf_size);
bool is_taking_input();
bool is_taking_char();
static int keyboard_input_character(char character);
void getchar(int row, int col, char *buffer);
static void keyboard_get_character(char character);
void keyboard_install();