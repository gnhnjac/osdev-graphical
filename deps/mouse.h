
#define VERTICAL_SCROLL_UP 1
#define VERTICAL_SCROLL_DOWN 0xF
#define WRITE_TO_MOUSE 0xD4
#define GET_MOUSE_ID 0xF2
#define MOUSE_RESET 0xFF
#define MOUSE_DEFAULT 0xF6
#define MOUSE_PACKET 0xF4
#define MOUSE_CHANGE_SAMPLE 0xF3

typedef struct
{

	char ascii;
	char attr;

} placeholder;

//refs
void mouse_wait(uint8_t type);
void disable_mouse();
void save_to_mouse_buffer();
void enable_mouse();
void mouse_handler();
void ack();
void mouse_write(uint8_t byte);
int identify();
void set_mouse_rate(int rate);
void mouse_install();