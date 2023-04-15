#include "keyboard.h"
#include "screen.h"
#include "mouse.h"
#include "shell.h"
#include "strings.h"
#include "low_level.h"
#include "heap.h"
#include "vfs.h"
#include <stdint.h>

uint32_t fid = 0; // current directory id

void shell_main()
{

	print("Please write your name: ");
	char name_buff[30];
	keyboard_input(-1,-1,name_buff,30);
	while(is_taking_input())
		continue;
	printf("\nHello, %s!",name_buff);

	while(true)
	{
		print("\n");
		pwd(fid);
		print("> ");
		char cmd_buff[300];
		keyboard_input(-1,-1,cmd_buff,300);
		while(is_taking_input())
			continue;
		handle_command(cmd_buff);
	}

}

void handle_command(char *cmd_buff)
{

	strip_from_start(cmd_buff, ' ');
	strip_from_end(cmd_buff, ' ');

	char *cmd = seperate_and_take(cmd_buff, ' ', 0);
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
	else if(strcmp(cmd, "ls"))
	{
		ls(fid);
	}
	else if(strcmp(cmd, "cd"))
	{
		handle_cd(cmd_buff);
	}
	else if(strcmp(cmd, "mkdir"))
	{
		handle_mkdir(cmd_buff);
	}
	else if(strcmp(cmd, "touch"))
	{
		handle_touch(cmd_buff);
	}
	else if(strcmp(cmd, "write"))
	{
		handle_write(cmd_buff);
	}
	else if(strcmp(cmd, "cat"))
	{
		handle_cat(cmd_buff);
	}
	else if(strcmp(cmd,"cls"))
	{
		clear_viewport();
	}
	else if(strcmp(cmd,"rm"))
	{
		handle_rm(cmd_buff);
	}
	else
	{

		printf("Unknown command '%s'. type 'help' for a list of available commands.",cmd);

	}

	free(cmd);

}

void handle_cd(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	fid = get_fid_by_name(param,fid);
	free(param);

	
}

void handle_mkdir(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	mkdir(param,fid);
	free(param);

	
}

void handle_touch(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	touch(param,fid);
	free(param);

	
}

void handle_write(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);

	clear_viewport();
	char *buff=malloc();
	keyboard_input(-1,-1,buff,HEAP_CHUNK_SIZE);
	cat_through_keyboard(get_fid_by_name(param,fid));
	while(is_taking_input())
		continue;
	write(get_fid_by_name(param,fid),buff);
	free(param);
	free(buff);
	
}

void handle_cat(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	cat(get_fid_by_name(param,fid));
	free(param);
	
}

void handle_rm(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	free_block(get_faddr_by_id(get_fid_by_name(param,fid))->bid);
	free(param);
	
}

char *help_strings[3] = {

	"Provides help for a certain command\nUsage: help CMD",
	"Reboots the computer.\nUsage: reboot",
	"Shuts down the computer\nUsage: shutdown",

};

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

		char *param = seperate_and_take(cmd_buff, ' ', 1);
		int str_index = get_help_index(param);
		if (str_index == -1)
		{

			printf("Help for command %s not found.",param);

		}
		else
		{
			printf("%s",help_strings[str_index]);
		}
		free(param);

	}
}

int get_help_index(char *cmd)
{

	if(strcmp(cmd,"help"))
		return 0;
	if(strcmp(cmd,"reboot"))
		return 1;
	if(strcmp(cmd,"shutdown"))
		return 2;
	return -1;

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