#include "keyboard.h"
#include "screen.h"
#include "mouse.h"
#include "shell.h"
#include "strings.h"
#include "low_level.h"

void shell_main()
{

	print("Please write your name: ");
	keyboard_input(-1,-1,30);
	while(is_taking_input())
		continue;
	char name_buff[30];
	load_input_to_buffer(name_buff);
	printf("\nHello, %s!",name_buff);

	while(true)
	{
		print("\n> ");
		keyboard_input(-1,-1,-1);
		while(is_taking_input())
			continue;
		char cmd_buff[INPUT_BUFFER_SIZE];
		load_input_to_buffer(cmd_buff);
		handle_command(cmd_buff);
	}

}

void handle_command(char *cmd_buff)
{

	strip_from_start(cmd_buff, ' ');
	strip_from_end(cmd_buff, ' ');

	char cmd[INPUT_BUFFER_SIZE];
	seperate_and_take(cmd_buff, cmd, ' ', 0);

	to_lower(cmd);

	if(strcmp(cmd, "help"))
	{
		handle_help(cmd_buff);
	}
	else if(strcmp(cmd, "reboot"))
	{
		handle_reboot();
	}
	else if(strcmp(cmd, "shutdown"))
	{
		handle_shutdown();
	}
	else
	{

		printf("Unknown command '%s'. type 'help' for a list of available commands.",cmd);

	}
}

void handle_help(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count > 2)
	{
		print("too many parameters.");
		return;
	}
	else if (param_count == 1)
	{
		print("Available commands:\nhelp\nreboot\nshutdown");
	}
	else
	{

		char param[INPUT_BUFFER_SIZE];
		seperate_and_take(cmd_buff, param, ' ', 1);

	}
}
 
void handle_reboot()
{
    uint8_t temp;
 
    asm volatile ("cli"); /* disable all interrupts */
    disable_mouse();
 
    /* Clear all keyboard buffers (output and command buffers) */
    do
    {
        temp = inb(KBRD_INTRFC); /* empty user data */
        if (check_flag(temp, KBRD_BIT_KDATA) != 0)
            inb(KBRD_IO); /* empty keyboard data */
    } while (check_flag(temp, KBRD_BIT_UDATA) != 0);
 
    outb(KBRD_INTRFC, KBRD_RESET); /* pulse CPU reset line */
loop:
    asm volatile ("hlt"); /* if that didn't work, halt the CPU */
    goto loop; /* if a NMI is received, halt again */
}

void handle_shutdown()
{

	#ifdef BOCHS
		outw(0xB004, 0x2000);
	#endif

	#ifdef QEMU
		outw(0x604, 0x2000);
	#endif
loop2:
    asm volatile ("hlt"); /* if that didn't work, halt the CPU */
    goto loop2; /* if a NMI is received, halt again */
}