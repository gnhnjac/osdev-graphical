#pragma once
#include <stdint.h>
#include "vmm.h"
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

struct _process {
   pdirectory*    pageDirectory;
   int            id;
   int            priority;
   int            state;
   uint32_t  imageBase;
   uint32_t  imageSize;
   terminal term;
   process* next;
   thread* threadList;
   char *name;
};

//refs
void removeProcessFromList(int id);
process *getProcessByID(int id);
int getFreeID();
int createProcess (char* exec);
void insert_process(process *proc);
void insert_thread_to_proc(process *proc, thread *t);
void terminateProcess ();
void clone_kernel_space(pdirectory* out);
void clone_kernel_stacks(pdirectory *out);
void clear_kernel_space(pdirectory *out);
void clear_kernel_stacks(pdirectory *out);
pdirectory* create_address_space ();
void print_processes();
bool does_process_own_term(process *p);
void printf(char *fmt,...);
void putchar(char c);
void print(char *msg);