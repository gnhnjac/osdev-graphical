#include "ps2.h"
#include "low_level.h"
void ps2_init()
{

	// flush ps2 buffer
	while (inb(PS_CTRL)&1) {inb(PS_DATA);}

}