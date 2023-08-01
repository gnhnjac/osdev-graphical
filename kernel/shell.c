#include "keyboard.h"
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
#include "window_sys.h"
#include "graphics.h"
#include "vmm.h"
#include "math.h"

char *path = 0; // current path

PWINDOW main_window = 0;
INPINFO window_input_info_struct;
PINPINFO window_input_info = 0;

#define SHELL_ROWS 25
#define SHELL_COLS 65

static void clear_viewport() // private
{

	gfx_clear_win(main_window);
	winsys_display_window(main_window);

}

void get_shell_input(char *buff, int buff_size)
{

	gfx_keyboard_input(window_input_info,-1,-1,buff,buff_size);

	while(window_input_info->is_taking_input)
	{
		if(main_window->event_handler.events[0].event_type & EVENT_INVALID)
			thread_suspend();

		EVENT e = winsys_dequeue_from_event_handler(&main_window->event_handler);

		char *kbdus = get_kbdus_char_array();

		char *shift_kbdus = get_kbdus_shift_char_array();

		if (e.event_type & EVENT_KBD_PRESS)
		{
			int pre_x = gfx_get_win_x(window_input_info->cursor_offset_x);
			int pre_y = gfx_get_win_y(window_input_info->cursor_offset_y);

			char c;
			char scancode = e.event_data&0xFF;

			if (e.event_data&0x200) // shift
				c = shift_kbdus[scancode];
			else
				c = kbdus[scancode];

			if (gfx_keyboard_input_character(window_input_info,c) != -1)
			{
				gfx_putchar(main_window, window_input_info, c);

				int post_x = gfx_get_win_x(window_input_info->cursor_offset_x);
				int post_y = gfx_get_win_y(window_input_info->cursor_offset_y);

				winsys_display_window_section(main_window,pre_x,pre_y,CHAR_WIDTH,CHAR_HEIGHT);
				winsys_display_window_section(main_window,post_x,post_y,CHAR_WIDTH,CHAR_HEIGHT);
				window_input_info->cursor_input_col = window_input_info->cursor_offset_x;
				window_input_info->cursor_input_row = window_input_info->cursor_offset_y;
			}

		}
	
	}

}

void free_path()
{

	kfree(path);

}

void shell_main()
{

	main_window = winsys_create_win(0,50,SHELL_COLS*CHAR_WIDTH,SHELL_ROWS*CHAR_HEIGHT, "shell", true);
	main_window->event_handler.event_mask |= GENERAL_EVENT_KBD;
	memset((char *)&window_input_info_struct,0,sizeof(INPINFO));
	window_input_info = &window_input_info_struct;

	process *proc = get_running_process();
	proc->term.term_win = main_window;
	proc->term.term_inp_info = window_input_info;

	proc->on_terminate = free_path;

	// (need signals and sigterm to handle that stuff instead of temporary on_terminate solution)
	path = kmalloc(3+1);

	strcpy(path,"a:\\");

	while(true)
	{
		printf("\n%s> ",path);
		char cmd_buff[300];
		get_shell_input(cmd_buff,300);
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
		kfree(cmd);
		handle_help(cmd_buff);
	}
	else if(strcmp(cmd, "reboot"))
	{
		kfree(cmd);
		handle_reboot();
	}
	else if(strcmp(cmd, "shutdown"))
	{
		kfree(cmd);
		handle_shutdown();
	}
	else if(strcmp(cmd, "ls"))
	{
		kfree(cmd);
		handle_ls();
	}
	else if(strcmp(cmd, "cd"))
	{
		kfree(cmd);
		handle_cd(cmd_buff);
	}
	else if(strcmp(cmd, "mkdir"))
	{
		kfree(cmd);
		handle_mkdir(cmd_buff);
	}
	else if(strcmp(cmd, "touch"))
	{
		kfree(cmd);
		handle_touch(cmd_buff);
	}
	else if(strcmp(cmd, "write"))
	{
		kfree(cmd);
		handle_write(cmd_buff);
	}
	else if(strcmp(cmd, "cat"))
	{
		kfree(cmd);
		handle_cat(cmd_buff);
	}
	else if(strcmp(cmd,"cls"))
	{
		kfree(cmd);
		clear_viewport();
		window_input_info->cursor_offset_y = 0;
		window_input_info->cursor_input_row = 0;
	}
	else if(strcmp(cmd,"rm"))
	{
		kfree(cmd);
		handle_rm(cmd_buff);
	}
	else if(strcmp(cmd,"size"))
	{
		kfree(cmd);
		handle_size(cmd_buff);
	}
	else if(strcmp(cmd,"concat"))
	{
		kfree(cmd);
		handle_concat(cmd_buff);
	}
	else if(strcmp(cmd,"stats"))
	{
		kfree(cmd);
		handle_stats();
	}
	else if(strcmp(cmd,"exec"))
	{
		kfree(cmd);
		handle_exec(cmd_buff);
	}
	else if(strcmp(cmd,"execbg"))
	{
		kfree(cmd);
		handle_execbg(cmd_buff);
	}
	else if(strcmp(cmd, "ps"))
	{
		kfree(cmd);
		print_processes();
	}
	else if(strcmp(cmd,"img"))
	{
		kfree(cmd);
		handle_img(cmd_buff);
	}
	else if(strcmp(cmd,"kill"))
	{
		kfree(cmd);
		handle_kill(cmd_buff);
	}
	else
	{

		printf("Unknown command '%s'. type 'help' for a list of available commands.",cmd);
		kfree(cmd);
	}

}

void handle_kill(char *cmd_buff)
{

	int param_count = count_substrings(cmd_buff, ' '); // including cmd
	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');
	
	uint32_t pid = decimal_to_uint(param);
	process *proc = getProcessByID(pid);

	if (proc)
	{
		if (proc->is_kernel)
			terminateKernelProcessById(pid);
		else
			terminateProcessById(pid);
	}
	kfree(param);

}

void handle_img(char *cmd_buff)
{
	int param_count = count_substrings(cmd_buff, ' '); // including cmd
	if (param_count != 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');
	char *new_path = join_path(path,param);

	gfx_open_bmp16(new_path,PIXEL_WIDTH/2,PIXEL_HEIGHT/2);;

	kfree(new_path);
	kfree(param);
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
	//print_mem_map();
	printf("\nHEAP:\n");
	print_heap_stats();
}

void handle_exec(char *cmd_buff)
{
	int param_count = count_substrings(cmd_buff, ' '); // including cmd
	if (param_count < 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');
	char *new_path = join_path(path,param);

	char *args = cmd_buff;

	while(*args && *args != ' ')
		args++;

	if (!(*args))
		args = 0;
	else
		args++;


	int pid = createProcess(new_path,args);

	kfree(new_path);
	kfree(param);

	waitForProcessToFinish(pid);
}

void handle_execbg(char *cmd_buff)
{
	int param_count = count_substrings(cmd_buff, ' '); // including cmd
	if (param_count < 2)
		return;

	char *param = seperate_and_take(cmd_buff, ' ', 1);
	strip_from_start(param, ' ');
	strip_from_end(param, ' ');
	char *new_path = join_path(path,param);

	char *args = cmd_buff;

	while(*args && *args != ' ')
		args++;

	if (!(*args))
		args = 0;
	else
		args++;


	int pid = createProcess(new_path,args);

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

char *help_strings[] = {

	"Provides help for a certain command\nUsage: help CMD",
	"Reboots the computer.\nUsage: reboot",
	"Shuts down the computer\nUsage: shutdown",
	"Lists the files in the current directory\nUsage: ls",
	"Changes directory to the specified directory\nUsage: cd DIR",
	"Creates a new directory in the current directory with the specified name\nUsage: mkdir PATH",
	"Makes a new file in the current directory with the specified name\nUsage: touch PATH",
	"Opens a text editor to write text to a file with the specified name, press esc to exit it\nUsage: write PATH\n2nd optional parameter is -o to override the previous contents of the file.",
	"Prints out the contents of a file\nUsage: cat PATH",
	"Clears the screen\nUsage: cls",
	"Removes a file or directory within the current directory with the specified name\nUsage: rm PATH",
	"Gives the size of the specified file in bytes\nUsage: size PATH",
	"Concatenates 2 files and stores them in a destination file\nUsage: concat DEST F1 F2",
	"Displays various stats\nUsage: stats",
	"Executes a PE program\nUsage: exec PATH",
	"Executes a PE program in the background\nUsage: execbg PATH",
	"Prints out the list of processes\nUsage: ps",
	"Opens a 16 color bmp image for viewing\nUsage: img PATH",
	"Kills the process with id PID\nUsage: kill PID",
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
		print("Available commands:\nhelp\nreboot\nshutdown\nls\ncd\nmkdir\ntouch\nwrite\ncat\ncls\nrm\nsize\nconcat\nstats\nexec\nexecbg\nps\nimg\nkill");
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
	if(strcmp(cmd,"stats"))
		return 13;
	if(strcmp(cmd,"exec"))
		return 14;
	if(strcmp(cmd,"execbg"))
		return 15;
	if(strcmp(cmd,"ps"))
		return 16;
	if(strcmp(cmd,"img"))
		return 17;
	if(strcmp(cmd,"kill"))
		return 18;

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