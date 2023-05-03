#include "keyboard.h"
#include "screen.h"
#include "mouse.h"
#include "shell.h"
#include "strings.h"
#include "low_level.h"
#include "heap.h"
#include "vfs.h"
#include "timer.h"
#include <stdint.h>

uint32_t fid = 0; // current directory id

void shell_main()
{

	print("Please write the time in the following format (hh:mm): ");
	char time_buff[5+1];
	keyboard_input(-1,-1,time_buff,6);
	while(is_taking_input())
		continue;
	
	strip_character(time_buff, ' ');
	time_buff[2] = 0; // null instead of :

	set_time(decimal_to_uint(time_buff),decimal_to_uint(time_buff+3));

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
	else if(strcmp(cmd,"size"))
	{
		handle_size(cmd_buff);
	}
	else if(strcmp(cmd,"concat"))
	{
		handle_concat(cmd_buff);
	}
	#ifdef BOCHS
		else if(strcmp(cmd,"paint"))
		{
			handle_paint(cmd_buff);
		}
	#endif
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
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	fid = get_fid_by_name(param,fid);
	free(param);

	
}

void handle_mkdir(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	mkdir(param,fid);
	free(param);

	
}

void handle_touch(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	if (get_fid_by_name(param,fid) != fid)
	{
		printf("file %s already exists",param);
		free(param);
		return;
	}

	touch(param,fid);
	free(param);

	
}

void handle_concat(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 4)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	if (get_fid_by_name(param,fid) != fid)
	{
		printf("file %s already exists",param);
		free(param);
		return;
	}

	char *param2 = seperate_and_take(cmd_buff, ' ', 2);
	strip_from_start(param2, ' ');
	strip_from_end(param2, ' ');

	uint32_t fid1 = get_fid_by_name(param2,fid);

	if (fid1 == fid)
	{
		printf("file %s doesn't exist",param2);
		free(param);
		free(param2);
		return;
	}

	char *param3 = seperate_and_take(cmd_buff, ' ', 3);
	strip_from_start(param3, ' ');
	strip_from_end(param3, ' ');

	uint32_t fid2 = get_fid_by_name(param3,fid);

	if (fid2 == fid)
	{
		printf("file %s doesn't exist",param3);
		free(param);
		free(param2);
		free(param3);
		return;
	}

	concat(param,fid1,fid2,fid);
	free(param);
	free(param2);
	free(param3);
	
}

void handle_write(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count < 2 || param_count > 3)
		return;

	int override = 0;

	if(param_count == 3)
	{

		char *option = seperate_and_take(cmd_buff, ' ', 2);

		if (strcmp(option, "-o"))
		{
			override = 1;
		}
		else
		{
			printf("unknown option %s", option);
			free(option);
			return;
		}

		free(option);

	}

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	clear_viewport();


	uint32_t file_fid = get_fid_by_name(param,fid);
	
	if(file_fid == fid)
	{
		handle_touch(cmd_buff);
		file_fid = get_fid_by_name(param,fid);
	}
	else if(!override)
	{
		cat(file_fid);
	}
	else
	{
		reset_file(file_fid);
	}

	char buff[1];

	do
	{
		getchar(-1,-1,buff);

		while(is_taking_char())
			continue;
 
		write(file_fid,buff, 0);

	} while (*buff != 27); // 27 is escape ascii

	free(param);
	
}

void handle_cat(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	uint32_t file_fid = get_fid_by_name(param,fid);

	if(file_fid == fid)
	{
		printf("file %s not found.", param);
		free(param);
		return;
	}

	cat(file_fid);
	free(param);
	
}

void handle_rm(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	free_block(get_faddr_by_id(get_fid_by_name(param,fid))->bid);
	free(param);
	
}

void handle_size(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	uint32_t file_fid = get_fid_by_name(param,fid);

	if(file_fid == fid)
	{
		printf("file %s not found.", param);
		free(param);
		return;
	}

	printf("file %s has size %d bytes",param,size(file_fid));
	free(param);
	
}

void handle_paint(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	disable_scrolling();
	clear_viewport();

	uint32_t file_fid = get_fid_by_name(param,fid);

	if(file_fid == fid) // didn't find file
	{
		handle_touch(cmd_buff);
		file_fid = get_fid_by_name(param,fid);
	}
	else
	{
		disable_mouse();
		int n = 0;
		for (int i = TOP; i < MAX_ROWS; i++)
		{

			for(int j = 0; j < MAX_COLS-2; j++)
			{

				char ascii = get_nth_char(file_fid, n);
				char attrib = get_nth_char(file_fid, n+1);
				print_char(ascii,i,j,attrib);

				n+=2;

			}

		}
		enable_mouse();
	}

	char buff[1];

	do
	{
		getchar(-1,-1,buff);

		while(is_taking_char())
			continue;

	} while (*buff != 27); // 27 is escape ascii

	disable_mouse();
	reset_file(file_fid);
	char *vidmem = (char *)VIDEO_ADDRESS;
	for (int i = TOP; i < MAX_ROWS; i++)
	{

		for(int j = 0; j < MAX_COLS-2; j++)
		{

			write(file_fid,vidmem+get_screen_offset(i,j), 1);
			write(file_fid,vidmem+get_screen_offset(i,j)+1, 1);

		}

	}
	clear_viewport();
	enable_scrolling();

	free(param);
	
}

char *help_strings[14] = {

	"Provides help for a certain command\nUsage: help CMD",
	"Reboots the computer.\nUsage: reboot",
	"Shuts down the computer\nUsage: shutdown",
	"Lists the files in the current directory\nUsage: ls",
	"Changes directory to the specified directory\nUsage: cd DIR",
	"Creates a new directory in the current directory with the specified name\nUsage: mkdir NAME",
	"Makes a new file in the current directory with the specified name\nUsage: touch NAME",
	"Opens a text editor to write text to a file with the specified name, press esc to exit it\nUsage: write NAME\n2nd optional parameter is -o to override the previous contents of the file.",
	"Prints out the contents of a file\nUsage: cat NAME",
	"Clears the screen\nUsage: cls",
	"Removes a file or directory within the current directory with the specified name\nUsage: rm NAME",
	"Gives the size of the specified file in bytes\nUsage: size NAME",
	"Concatenates 2 files and stores them in a destination file\nUsage: concat DEST F1 F2",
	"Opens a paint editor, press esc to exit it, saves it with the specified name\nUsage: paint NAME"

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
		print("Available commands:\nhelp\nreboot\nshutdown\nls\ncd\nmkdir\ntouch\nwrite\ncat\ncls\nrm\nsize\nconcat\npaint");
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
	if(strcmp(cmd,"ls"))
		return 3;
	if(strcmp(cmd,"cd"))
		return 4;
	if(strcmp(cmd,"mkdir"))
		return 5;
	if(strcmp(cmd,"touch"))
		return 6;
	if(strcmp(cmd,"write"))
		return 7;
	if(strcmp(cmd,"cat"))
		return 8;
	if(strcmp(cmd,"cls"))
		return 9;
	if(strcmp(cmd,"rm"))
		return 10;
	if(strcmp(cmd,"size"))
		return 11;
	if(strcmp(cmd,"concat"))
		return 12;
	if(strcmp(cmd,"paint"))
		return 13;

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