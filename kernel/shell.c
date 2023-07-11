#include "keyboard.h"
#include "screen.h"
#include "mouse.h"
#include "shell.h"
#include "strings.h"
#include "low_level.h"
#include "heap.h"
#include "vfs.h"
#include "timer.h"
#include "pmm.h"
#include "memory.h"
#include "heap.h"
#include <stdint.h>
#include "process.h"
#include "scheduler.h"



#include "vmm.h"

char *path = 0; // current path

void shell_main()
{

	path = kmalloc(3+1);

	strcpy(path,"a:\\");

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
		printf("%s",path);
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
		//ls(fid);
		handle_ls();
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
		disable_mouse();
		clear_viewport();
		set_cursor_input_row(TOP);
		set_cursor_row(TOP);
		enable_mouse();
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
	else if(strcmp(cmd,"stats"))
	{
		handle_stats();
	}
	else if(strcmp(cmd,"paint"))
	{
		handle_paint(cmd_buff);
	}
	else if(strcmp(cmd,"exec"))
	{
		handle_exec(cmd_buff);
	}
	else if(strcmp(cmd, "ps"))
	{
		print_processes();
	}
	else if(strcmp(cmd, "pt"))
	{
		print_threads();
	}
	else
	{

		printf("Unknown command '%s'. type 'help' for a list of available commands.",cmd);

	}

	kfree(cmd);

}

char *join_path(char* p, char* ext)
{

	char *new_path;

	if (count_substrings(ext,'\\') == 1)
	{
		new_path = kmalloc(strlen(ext)+strlen(p)+1+((p[strlen(p)-1] == '\\') ? 0 : 1));
		strcpy(new_path,p);
		strcpy(new_path+strlen(p)+((p[strlen(p)-1] == '\\') ? 0 : 1),ext);

		if (p[strlen(p)-1] != '\\')
			new_path[strlen(p)] = '\\';
	}
	else
	{
		new_path = kmalloc(strlen(ext)+1);
		strcpy(new_path,ext);
	}

	return new_path;

}

void handle_stats()
{

	printf("PMM:\nTotal memory: %dKb\nTotal Blocks: %d\nUsed Blocks: %d\nFree Blocks: %d\n",pmmngr_get_memory_size(),pmmngr_get_block_count(),pmmngr_get_used_blocks(),pmmngr_get_free_block_count());
	print_mem_map();
	printf("\nHEAP:\n");
	print_heap_stats();
}

void handle_exec(char *cmd_buff)
{
	int param_count = count_substrings(cmd_buff, ' '); // including cmd
	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');
	char *new_path = join_path(path,param);

	int pid = createProcess(new_path);

	kfree(new_path);
	kfree(param);
}

void handle_ls()
{

	PFILELIST lst = volOpenDir(path);

	uint32_t sz = 0;
	uint32_t fls = 0;
	uint32_t dirs = 0;

	while (lst)
	{
		printf("%s    %d %d-%d-%d\n",lst->f.name,lst->f.fileLength,lst->f.DateCreated&0x1F,(lst->f.DateCreated&0x1F0)>>5,(lst->f.DateCreated&(0b1111111<<9))>>10);

		sz += lst->f.fileLength;

		if (lst->f.flags == FS_DIRECTORY)
			dirs++;
		else
			fls++;

		volCloseFile(&lst->f);

		PFILELIST tmp = lst->next;

		kfree(lst);

		lst = tmp;

	}

	printf("%d File(s)    %d Bytes.\n",fls,sz);
	printf("%d Dir(s)",dirs);

}

void handle_cd(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	char *new_path = join_path(path,param);
	FILE f = volOpenFile(new_path);

	if (f.flags == FS_DIRECTORY){
		kfree(path);
		path = new_path;
	}
	else
		kfree(new_path);

	volCloseFile(&f);
	kfree(param);
	
}

void handle_mkdir(char *cmd_buff)
{
	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	char *new_path = join_path(path,param);

	char *dir_name = 0;

	for (int i = strlen(new_path)-1; i >= 0; i--)
	{

		if (new_path[i] == '\\')
		{
			dir_name = new_path+i+1;
			new_path[i] = 0;
			break;
		}

	}

	FILE fold = volOpenFile(new_path);

	if (fold.flags == FS_DIRECTORY){
		volCreateFile(&fold, dir_name, FS_DIRECTORY);
	}

	volCloseFile(&fold);

	kfree(param);
	kfree(new_path);

	
}

void handle_touch(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	char *new_path = join_path(path,param);

	char *f_name = 0;

	for (int i = strlen(new_path)-1; i >= 0; i--)
	{

		if (new_path[i] == '\\')
		{
			f_name = new_path+i+1;
			new_path[i] = 0;
			break;
		}

	}

	FILE fold = volOpenFile(new_path);

	if (fold.flags == FS_DIRECTORY){
		volCreateFile(&fold, f_name, FS_FILE);
	}

	volCloseFile(&fold);

	kfree(param);
	kfree(new_path);
	
}

void handle_concat(char *cmd_buff)
{

	// int param_count = count_substrings(cmd_buff, ' '); // including cmd

	// if (param_count != 4)
	// 	return;

	// char *param = seperate_and_take(cmd_buff, ' ', 1);
	// strip_from_start(param, ' ');
	// strip_from_end(param, ' ');

	// if (get_fid_by_name(param,fid) != fid)
	// {
	// 	printf("file %s already exists",param);
	// 	kfree(param);
	// 	return;
	// }

	// char *param2 = seperate_and_take(cmd_buff, ' ', 2);
	// strip_from_start(param2, ' ');
	// strip_from_end(param2, ' ');

	// uint32_t fid1 = get_fid_by_name(param2,fid);

	// if (fid1 == fid)
	// {
	// 	printf("file %s doesn't exist",param2);
	// 	kfree(param);
	// 	kfree(param2);
	// 	return;
	// }

	// char *param3 = seperate_and_take(cmd_buff, ' ', 3);
	// strip_from_start(param3, ' ');
	// strip_from_end(param3, ' ');

	// uint32_t fid2 = get_fid_by_name(param3,fid);

	// if (fid2 == fid)
	// {
	// 	printf("file %s doesn't exist",param3);
	// 	kfree(param);
	// 	kfree(param2);
	// 	kfree(param3);
	// 	return;
	// }

	// concat(param,fid1,fid2,fid);
	// kfree(param);
	// kfree(param2);
	// kfree(param3);
	
}

void handle_write(char *cmd_buff)
{

	// int param_count = count_substrings(cmd_buff, ' '); // including cmd

	// if (param_count < 2 || param_count > 3)
	// 	return;

	// int override = 0;

	// if(param_count == 3)
	// {

	// 	char *option = seperate_and_take(cmd_buff, ' ', 2);

	// 	if (strcmp(option, "-o"))
	// 	{
	// 		override = 1;
	// 	}
	// 	else
	// 	{
	// 		printf("unknown option %s", option);
	// 		kfree(option);
	// 		return;
	// 	}

	// 	kfree(option);

	// }

	// char *param = seperate_and_take(cmd_buff, ' ', 1);
	// strip_from_start(param, ' ');
	// strip_from_end(param, ' ');

	// clear_viewport();


	// uint32_t file_fid = get_fid_by_name(param,fid);
	
	// if(file_fid == fid)
	// {
	// 	handle_touch(cmd_buff);
	// 	file_fid = get_fid_by_name(param,fid);
	// }
	// else if(!override)
	// {
	// 	cat(file_fid);
	// }
	// else
	// {
	// 	reset_file(file_fid);
	// }

	// char buff[1];

	// do
	// {
	// 	getchar(-1,-1,buff);

	// 	while(is_taking_char())
	// 		continue;
 
	// 	write(file_fid,buff, 0);

	// } while (*buff != 27); // 27 is escape ascii

	// kfree(param);
	
}

void handle_cat(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	char *file_path = join_path(path,param);
	FILE f = volOpenFile(file_path);

	if (f.flags == FS_FILE)
	{
		char *buff = "x";

		while (!f.eof)
		{
			volReadFile(&f,buff,1);
			printf("%c",*buff);
		}
	}

	volCloseFile(&f);
	kfree(param);
	kfree(file_path);
	
}

void handle_rm(char *cmd_buff)
{

	// int param_count = count_substrings(cmd_buff, ' '); // including cmd

	// if (param_count != 2)
	// 	return;

	// char *param = seperate_and_take(cmd_buff, ' ', 1);
	// strip_from_start(param, ' ');
	// strip_from_end(param, ' ');

	// free_block(get_faddr_by_id(get_fid_by_name(param,fid))->bid);
	// kfree(param);
	
}

void handle_size(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd

	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');

	char *file_path = join_path(path,param);
	FILE f = volOpenFile(file_path);

	if(f.flags != FS_INVALID)
	{
		printf("file %s has size %d bytes",f.name,f.fileLength);
	}

	volCloseFile(&f);
	kfree(param);
	kfree(file_path);
	
}

void handle_paint(char *cmd_buff)
{

	// int param_count = count_substrings(cmd_buff, ' '); // including cmd

	// if (param_count != 2)
	// 	return;

	// char *param = seperate_and_take(cmd_buff, ' ', 1);
	// strip_from_start(param, ' ');
	// strip_from_end(param, ' ');

	// disable_scrolling();
	// clear_viewport();

	// uint32_t file_fid = get_fid_by_name(param,fid);

	// if(file_fid == fid) // didn't find file
	// {
	// 	handle_touch(cmd_buff);
	// 	file_fid = get_fid_by_name(param,fid);
	// }
	// else
	// {
	// 	disable_mouse();
	// 	int n = 0;
	// 	for (int i = TOP; i < MAX_ROWS; i++)
	// 	{

	// 		for(int j = 0; j < MAX_COLS-2; j++)
	// 		{

	// 			char ascii = get_nth_char(file_fid, n);
	// 			char attrib = get_nth_char(file_fid, n+1);
	// 			print_char(ascii,i,j,attrib);

	// 			n+=2;

	// 		}

	// 	}
	// 	enable_mouse();
	// }

	// char buff[1];

	// do
	// {
	// 	getchar(-1,-1,buff);

	// 	while(is_taking_char())
	// 		continue;

	// } while (*buff != 27); // 27 is escape ascii

	// disable_mouse();
	// reset_file(file_fid);
	// char *vidmem = (char *)VIDEO_ADDRESS;
	// for (int i = TOP; i < MAX_ROWS; i++)
	// {

	// 	for(int j = 0; j < MAX_COLS-2; j++)
	// 	{

	// 		write(file_fid,vidmem+get_screen_offset(i,j), 1);
	// 		write(file_fid,vidmem+get_screen_offset(i,j)+1, 1);

	// 	}

	// }
	// clear_viewport();
	// enable_scrolling();
	// enable_mouse();

	// kfree(param);
	
}

char *help_strings[15] = {

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
	"Opens a paint editor, press esc to exit it, saves it with the specified name\nUsage: paint NAME",
	"Displays various stats\nUsage: stats",
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
		print("Available commands:\nhelp\nreboot\nshutdown\nls\ncd\nmkdir\ntouch\nwrite\ncat\ncls\nrm\nsize\nconcat\npaint\nstats");
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
		kfree(param);

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
	if(strcmp(cmd,"stats"))
		return 14;

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