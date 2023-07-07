#include "process.h"
#include "image.h"
#include "vmm.h"
#include "memory.h"
#include "loader.h"
#include "heap.h"
#include "tss.h"
#include "screen.h"
#include "scheduler.h"
#include "pmm.h"

#define PAGE_SIZE 4096
#define PROC_INVALID_ID -1

static process *processList = 0;

static process *running_process = 0;

void removeProcessFromList(int id)
{

    process *tmp = processList;
    process *prev = 0;

    while (tmp)
    {

        if (tmp->id == id)
        {
            if (prev)
                prev->next = tmp->next;
            else
                processList = tmp->next;
            return;
        }

        prev = tmp;
        tmp = tmp->next;

    }

}

process  *getRunningProcess()
{

    return running_process;

}

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

int getFreeID()
{

    int id = 1;

    process *tmp = processList;

    while (tmp)
    {

        if (tmp->id == id)
            id++;

        tmp = tmp->next;

    }

    return id;

}

int createProcess (char* exec) {
    
    pdirectory *prevDir = vmmngr_get_directory();

    /* get process virtual address space */
    pdirectory *addressSpace = create_address_space(); // *** REMEMBER TO FREE IT ONCE THE PROCESS TERMINATES

    disable_scheduling();
    vmmngr_switch_pdirectory(addressSpace);
    PImageInfo imageInfo = load_executable(addressSpace,exec);

    if (!imageInfo)
    {
        pmmngr_free_block(addressSpace);
        vmmngr_switch_pdirectory(prevDir);
        enable_scheduling();
        return 0;
    }

    /* create PCB */

    process *proc = (process *)kmalloc(sizeof(process));

    proc->id            = getFreeID();
    proc->pageDirectory = addressSpace;
    proc->priority      = 1;
    proc->state         = PROCESS_STATE_ACTIVE;
    proc->next = 0;
    proc->imageBase = imageInfo->ImageBase;
    proc->imageSize = imageInfo->ImageSize;

    /* Create userspace stack (4k size) */
    void* stack = (void*) (imageInfo->ImageBase + imageInfo->ImageSize + PAGE_SIZE);

    if ((uint32_t)stack % PAGE_SIZE != 0)
        stack += PAGE_SIZE - (uint32_t)stack % PAGE_SIZE;

    /* map user process stack space */
    vmmngr_alloc_virt (addressSpace, stack - PAGE_SIZE, // stack grows downwards
    I86_PDE_WRITABLE|I86_PDE_USER,
	I86_PTE_WRITABLE|I86_PTE_USER);

    /* create thread descriptor */
    thread *mainThread       = (thread *)kmalloc(sizeof(thread));
    thread_create(mainThread,(void *)(imageInfo->EntryPointRVA + imageInfo->ImageBase),stack, false);

    vmmngr_switch_pdirectory(prevDir);
    enable_scheduling();

    mainThread->parent = proc;
    mainThread->initialStack = stack;

    kfree(imageInfo);

    insert_process(proc);

    queue_insert(*mainThread);

    kfree(mainThread);

    insert_thread_to_proc(proc,queue_get_last());

    return proc->id;
}

void insert_process(process *proc)
{

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

}

void insert_thread_to_proc(process *proc, thread *t)
{

    thread *tmp = proc->threadList;

    if (tmp)
    {
        while (tmp->next)
            tmp = tmp->next;

        tmp->next = t;
    }
    else
        proc->threadList = t;

}

void terminateProcess () {

    process *proc = running_process;
    if (!proc)
            return;
    if (proc->id==PROC_INVALID_ID)
            return;

    /* release threads */
    thread* pThread = proc->threadList;


    uint32_t fileSize = proc->imageSize;
    if (fileSize % 4096 != 0)
        fileSize += 4096 - fileSize % 4096;

    /* unmap and release image memory */
    for (uint32_t virt = proc->imageBase; virt < proc->imageBase+fileSize; virt+=4096) {

        /* unmap and release page */
        vmmngr_free_virt (proc->pageDirectory, (void *)virt);
    }

    while (pThread)
    {

        // unmap virtual stack
        vmmngr_free_virt (proc->pageDirectory, (void *) pThread->initialStack-PAGE_SIZE); // stack is 4k
        thread *tmp = pThread;
        pThread = pThread->next;
        remove_by_tid(tmp->tid); // remove process from process list

    }

   running_process = 0;

   printf("\nProcess %d terminated.\n",proc->id);

   removeProcessFromList(proc->id);

   __asm__("sti");

   schedule(); // force task switch.

}

/* clones kernel space into new address space. */
void clone_kernel_space(pdirectory* out) {

    /* get current process directory. */
    pdirectory* proc = vmmngr_get_directory();

    /* copy kernel page tables into this new page directory.
    Recall that KERNEL SPACE is 0xc0000000, which starts at
    entry 768. */
    memcpy((char *)&out->m_entries[768], (char *)&proc->m_entries[768], 256 * sizeof (pd_entry));
    
    // also copy first 4mb
    memcpy((char *)&out->m_entries[0], (char *)&proc->m_entries[0], 1 * sizeof (pd_entry));
}

/* create new address space. */
pdirectory* create_address_space () {

    pdirectory* space =  vmmngr_create_pdir();

    /* clone kernel space. */
    clone_kernel_space(space);
    return space;

}

void print_processes()
{

    process *tmp = processList;

    while (tmp)
    {

        printf("name: %s, pid: %d\n",tmp->name, tmp->id);

        thread *tmp_thread = tmp->threadList;

        while (tmp_thread)
        {

            printf("tid: %d, state: %b\n",tmp_thread->tid, tmp_thread->state);

            tmp_thread = tmp_thread->next;

        }

        tmp = tmp->next;

    }

}