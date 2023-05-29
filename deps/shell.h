/* keyboard interface IO port: data and control
   READ:   status port
   WRITE:  control register */
#define KBRD_INTRFC 0x64
 
/* keyboard interface bits */
#define KBRD_BIT_KDATA 0 /* keyboard data is in buffer (output buffer is empty) (bit 0) */
#define KBRD_BIT_UDATA 1 /* user data is in buffer (command buffer is empty) (bit 1) */
 
#define KBRD_IO 0x60 /* keyboard IO port */
#define KBRD_RESET 0xFE /* reset CPU command */

#define bit(n) (1<<(n)) /* Set bit n to 1 */

/* Check if bit n in flags is set */
#define check_flag(flags, n) ((flags) & bit(n))

//refs
void shell_main();
void handle_command(char *cmd_buff);
void handle_stats();
void handle_ls();
void handle_cd(char *cmd_buff);
void handle_mkdir(char *cmd_buff);
void handle_touch(char *cmd_buff);
void handle_concat(char *cmd_buff);
void handle_write(char *cmd_buff);
void handle_cat(char *cmd_buff);
void handle_rm(char *cmd_buff);
void handle_size(char *cmd_buff);
void handle_paint(char *cmd_buff);
void handle_help(char *cmd_buff);
int get_help_index(char *cmd);
void handle_reboot();
void handle_shutdown();