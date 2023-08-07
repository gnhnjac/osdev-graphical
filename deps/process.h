#pragma once
#include <stdint.h>
#include "vmm.h"
#include "vfs.h"
#include "window_sys.h"

#define PROCESS_STATE_SLEEP  0
#define PROCESS_STATE_ACTIVE 1
#define MAX_THREAD 5
#define PROC_INVALID_ID -1

typedef struct _trapFrame {
   /* pushed by isr. */
   uint32_t gs;
   uint32_t fs;
   uint32_t es;
   uint32_t ds;
   /* pushed by pushf. */
   uint32_t edi;
   uint32_t esi;
   uint32_t ebp;
   uint32_t esp; // skipped in pop but need to include it as null int
   uint32_t ebx;
   uint32_t edx;
   uint32_t ecx;
   uint32_t eax;
   /* pushed by cpu. */
   uint32_t eip;
   uint32_t cs;
   uint32_t flags;
}trapFrame;

typedef struct _userTrapFrame
{

   /* pushed by isr. */
   uint32_t gs;
   uint32_t fs;
   uint32_t es;
   uint32_t ds;
   /* pushed by pushf. */
   uint32_t edi;
   uint32_t esi;
   uint32_t ebp;
   uint32_t esp; // skipped in pop but need to include it as null int
   uint32_t ebx;
   uint32_t edx;
   uint32_t ecx;
   uint32_t eax;
   /* pushed by cpu. */
   uint32_t eip;
   uint32_t cs;
   uint32_t flags;
   uint32_t user_stack;
   uint32_t user_ss;

}userTrapFrame;

typedef struct _process process;
typedef struct _thread thread;

struct _thread {
   uint32_t    ESP;
   uint32_t    SS;
   uint32_t    kernelESP;
   uint32_t    kernelSS;
   process*  parent;
   uint32_t    priority;
   int         state;
   uint32_t     sleepTimeDelta;
   void*     initialStack;
   thread *next;
   int tid;
   bool isMain;
};

typedef struct _file_desc
{

   FILE file;
   uint32_t fd;
   struct _file_desc *next;

} FILE_DESC, *PFILE_DESC;

struct _process {
   pdirectory*    pageDirectory;
   int            id;
   int            priority;
   int            state;
   uint32_t  imageBase;
   uint32_t  imageSize;
   uintptr_t brk;
   PFILE_DESC file_descs;
   terminal term;
   process* next;
   thread* threadList;
   uint32_t livingThreads;
   bool is_kernel;
   void (*on_terminate)();
   char *name;
};

#define SBRK_LIM 0x40000000

//refs
void removeProcessFromList(int id);
process *getProcessByID(int id);
int getFreeID();
int createKernelProcess(void *entry, char *name);
void terminateKernelProcessById (int pid);
int createProcess(char* exec, char *args);
uintptr_t inc_proc_brk(uintptr_t inc);
PFILE get_file_by_fd(process *proc, uint32_t fd);
uint32_t append_fd_to_process(process *proc, FILE f);
void close_fd(process *proc, uint32_t fd);
uint32_t fopen(char *path);
void fread(uint32_t fd, unsigned char* Buffer, unsigned int Length);
void fclose(uint32_t fd);
void insert_process(process *proc);
void insert_thread_to_proc(process *proc, thread *t);
void waitForProcessToFinish(int pid);
void terminateProcess();
void terminateProcessById (int pid);
void clone_kernel_space(pdirectory* out);
void clone_kernel_stacks(pdirectory *out);
void clear_kernel_space(pdirectory *out);
void clear_kernel_stacks(pdirectory *out);
pdirectory* create_address_space ();
void print_processes();
bool does_process_own_term(process *p);
void printf(char *fmt,...);
void vprintf(char *fmt,va_list valist);
void printf_term(terminal term, char *fmt,...);
void putchar(char c);
void print(char *msg);