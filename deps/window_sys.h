#include <stdint.h>

struct _window
{

	uint32_t w_width;
	uint32_t w_height;
	void *w_buffer;
	char *w_name;



} WINDOW, *PWINDOW;

//refs