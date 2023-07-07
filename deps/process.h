#pragma once
#include <stdint.h>
#include "vmm.h"

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
   /* used only when coming from/to user mode. */
// uint32_t user_stack;
// uint32_t user_ss;
}trapFrame;

typedef struct _process process;
typedef struct _thread thread;

typedef struct _thread {
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
};

typedef struct _process {
   int            id;
   int            priority;
   pdirectory*    pageDirectory;
   int            state;
   uint32_t  imageBase;
   uint32_t  imageSize;
   process* next;
   thread* threadList;
   char *name;
};

//refs
void removeProcessFromList(int id);
process  *getRunningProcess();
process *getProcessByID(int id);
int getFreeID();
int createProcess (char* exec);
void insert_process(process *proc);
void insert_thread_to_proc(process *proc, thread *t);
void terminateProcess ();
void clone_kernel_space(pdirectory* out);
pdirectory* create_address_space ();
void print_processes();