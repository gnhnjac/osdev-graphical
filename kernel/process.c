#include "process.h"
#include "image.h"
#include "vmm.h"
#include "memory.h"
#include "loader.h"
#include "heap.h"
#include "tss.h"

#define PAGE_SIZE 4096
#define PROC_INVALID_ID -1

static process *processList = 0;

static process *running_process = 0;

process *getProcessByID(int id)
{

    process *tmp = processList;

    while (tmp)
    {

        if (tmp->id == id)
            return tmp;

        tmp = tmp->next;

    }

    return 0;

}

int createProcess (char* exec) {
    
    pdirectory *prevDir = vmmngr_get_directory();

    /* get process virtual address space */
    //addressSpace = vmmngr_createAddressSpace ();
    pdirectory *addressSpace = vmmngr_create_pdir();
    memcpy((void *)addressSpace, (void *)vmmngr_get_directory(),4096);
    vmmngr_switch_pdirectory(addressSpace);
    PImageInfo imageInfo = load_executable(addressSpace,exec);
    if (!imageInfo)
    {
        vmmngr_switch_pdirectory(prevDir);
        return 0;
    }

    /* create PCB */

    process *proc = (process *)kmalloc(sizeof(process));

    proc->id            = 1;
    proc->pageDirectory = addressSpace;
    proc->priority      = 1;
    proc->state         = PROCESS_STATE_ACTIVE;
    proc->threadCount   = 1;
    proc->next = 0;

    /* create thread descriptor */
    thread *mainThread       = (thread *)kmalloc(sizeof(thread));
    proc->threadList = mainThread;
    mainThread->kernelStack  = 0;
    mainThread->parent       = proc;
    mainThread->priority     = 1;
    mainThread->state        = PROCESS_STATE_ACTIVE;
    mainThread->initialStack = 0;
    mainThread->stackLimit   = (void*) ((uint32_t) mainThread->initialStack + 4096);
    mainThread->imageBase    = imageInfo->ImageBase;
    mainThread->imageSize    = imageInfo->ImageSize;
    memset ((char *)&mainThread->frame, 0, sizeof (trapFrame));
    mainThread->frame.eip    = imageInfo->EntryPointRVA + imageInfo->ImageBase;
    mainThread->frame.flags  = 0x200;
    mainThread->next = 0;

    /* Create userspace stack (4k size) */
    void* stack = (void*) (imageInfo->ImageBase + imageInfo->ImageSize + PAGE_SIZE);

    if ((uint32_t)stack % PAGE_SIZE != 0)
        stack += PAGE_SIZE - (uint32_t)stack % PAGE_SIZE;

    /* map user process stack space */
    vmmngr_alloc_virt (addressSpace, stack - PAGE_SIZE, // stack grows downwards
    I86_PDE_WRITABLE|I86_PDE_USER,
	I86_PTE_WRITABLE|I86_PTE_USER);

    /* final initialization */
    mainThread->initialStack = stack;
    mainThread->frame.esp    = (uint32_t)mainThread->initialStack;
    mainThread->frame.ebp    = mainThread->frame.esp;

    kfree(imageInfo);

    if (processList == 0)
    {
        processList = proc;
    }
    else
    {

        process *tmp = processList;
        while (tmp->next != 0)
            tmp = tmp->next;
        tmp->next = proc;

    }

    vmmngr_switch_pdirectory(prevDir);

    return proc->id;
}

void executeProcess (int id) {
        int entryPoint = 0;
        unsigned int procStack = 0;

        /* get running process */
        process* proc = getProcessByID(id);

        if (!proc)
            return;
        if (proc->id==PROC_INVALID_ID)
                return;
        if (!proc->pageDirectory)
                        return;

        /* get esp and eip of main thread */
        entryPoint = proc->threadList->frame.eip;
        procStack  = proc->threadList->frame.esp;

        running_process = proc;

        /* switch to process address space */
        __asm__ ("cli");
        vmmngr_switch_pdirectory (proc->pageDirectory);

        int stack=0;
        __asm__ ("mov %%esp,%%eax" : "=esp" ( stack ));

        tss_set_stack(0x10,stack);

        /* execute process in user mode */
        __asm__ (
                "mov     $0x23,%%ax\n"
                "mov     %%ax, %%ds\n"
                "mov     %%ax, %%es\n"
                "mov     %%ax, %%fs\n"
                "mov     %%ax, %%gs\n"
                "push   $0x23\n"
                "push   %0\n" : : "m" (procStack));

        __asm__ (
                "push    $0x200\n"
                "push    $0x1b\n"
                "push   %0\n"
                "iret\n" : : "m" (entryPoint)
        );

}

void terminateProcess () {
    process *proc = running_process;
    if (!proc)
            return;
    if (proc->id==PROC_INVALID_ID)
            return;

    /* release threads */
    thread* pThread = proc->threadList;

    // main thread
    uint32_t fileSize = pThread->imageSize;
    if (fileSize % 4096 != 0)
        fileSize += 4096 - fileSize % 4096;

    /* unmap and release image memory */
    for (uint32_t virt = pThread->imageBase; virt < pThread->imageBase+fileSize; virt+=4096) {

        /* unmap and release page */
        vmmngr_free_virt (proc->pageDirectory, (void *)virt);
    }

    while (pThread)
    {

        // unmap virtual stack
        vmmngr_free_virt (proc->pageDirectory, (void *) pThread->initialStack-PAGE_SIZE); // stack is 4k

        pThread = pThread->next;

    }

    // restore data segment selectors
   __asm__ (
            "cli\n"
            "mov     $0x10,%ax\n"
            "mov     %ax, %ds\n"
            "mov     %ax, %es\n"
            "mov     %ax, %fs\n"
            "mov     %ax, %gs\n"
            "sti");

    __asm__ ("add $0x70, %esp"); 
    // very very hacky, what's needed is to add a stub at the end of the program that it will return to it,
    // allocate that stub in user space and make it call terminate process instead of the process itself calling it

    running_process = 0;

    while(1);

}