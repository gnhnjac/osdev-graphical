#include <stdint.h>
#include "vmm.h"

#define PROCESS_STATE_SLEEP  0
#define PROCESS_STATE_ACTIVE 1
#define MAX_THREAD 5
#define PROC_INVALID_ID -1

typedef struct _trapFrame {
   uint32_t esp;
   uint32_t ebp;
   uint32_t eip;
   uint32_t edi;
   uint32_t esi;
   uint32_t eax;
   uint32_t ebx;
   uint32_t ecx;
   uint32_t edx;
   uint32_t flags;
   /*
      note: we can add more registers to this.
      For a complete trap frame, you should add:
        -Debug registers
        -Segment registers
        -Error condition [if any]
        -v86 mode segment registers [if used]
   */
}trapFrame;

typedef struct _process process;
typedef struct _thread thread;

typedef struct _thread {
   process*  parent;
   void*     initialStack;
   void*     stackLimit;
   void*     kernelStack;
   uint32_t  priority;
   int       state;
   trapFrame frame;
   uint32_t  imageBase;
   uint32_t  imageSize;
   thread *next;
};

typedef struct _process {
   int            id;
   int            priority;
   pdirectory*    pageDirectory;
   int            state;
   process* next;
   int threadCount;
   thread* threadList;

   // temporary variables
   int prevEBP;
   int prevEIP;
};

//refs
process *getProcessByID(int id);
int createProcess (char* exec);
void executeProcess (int id);
void terminateProcess ();