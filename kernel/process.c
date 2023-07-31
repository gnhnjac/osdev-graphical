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
#include "graphics.h"
#include "math.h"
#include "strings.h"

#define PAGE_SIZE 4096
#define PROC_INVALID_ID -1

static process *processList = 0;

void removeProcessFromList(int id)
{

    process *tmp = processList;

    if (tmp->next == 0)
    {

        if (tmp->id == id)
            processList = 0;
        return;

    }

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
        if (tmp->id >= id)
                id=tmp->id+1;

        tmp = tmp->next;

    }

    return id;

}

int createKernelProcess(void *entry, char *name)
{

    pdirectory *prevDir = vmmngr_get_directory();

    process *kernel_proc = (process *)kcalloc(sizeof(process));

    kernel_proc->id            = getFreeID();
    kernel_proc->pageDirectory = create_address_space();
    // temporarily copy stack to new address space
    void *running_proc_stack = (void *)get_current_task()->kernelESP-PAGE_SIZE;
    vmmngr_mmap_virt2virt(prevDir,kernel_proc->pageDirectory,running_proc_stack,running_proc_stack,I86_PDE_WRITABLE,I86_PTE_WRITABLE);
    
    disable_scheduling();
    vmmngr_switch_pdirectory(kernel_proc->pageDirectory);
    kernel_proc->priority      = PRIORITY_HIGH;
    kernel_proc->state         = PROCESS_STATE_ACTIVE;
    kernel_proc->next = 0;
    kernel_proc->threadList = 0;
    kernel_proc->livingThreads = 0;
    kernel_proc->name = kmalloc(strlen(name)+1);
    strcpy(kernel_proc->name,name);
    kernel_proc->is_kernel = true;
    insert_process(kernel_proc);

    /* create main thread and add it. */
    thread *main_thread = (thread *)kcalloc(sizeof(thread));
    thread_create(main_thread, entry, create_kernel_stack(), true);
    vmmngr_switch_pdirectory(prevDir);
    enable_scheduling();
    vmmngr_unmap_virt(kernel_proc->pageDirectory,running_proc_stack);

    main_thread->parent = kernel_proc;
    main_thread->isMain = true;
    main_thread->priority = PRIORITY_HIGH;
    queue_insert(*main_thread);
    insert_thread_to_proc(kernel_proc,main_thread);

    return kernel_proc->id;

}

void terminateKernelProcessById (int pid) {

    // NEED TO DO THIS WITH SIGNALS SO PROCESS ITSELF WILL RUN THIS

    process *proc = getProcessByID(pid);

    if (!proc)
        return;

    void (*on_terminate)();

    on_terminate = proc->on_terminate;

    if (on_terminate)
        on_terminate();

    disable_scheduling();

    /* release threads */
    thread* pThread = proc->threadList;

    while (pThread)
    {

        thread *tmp = pThread;
        pThread = pThread->next;
        remove_by_tid(tmp->tid); // remove thread from thread list
        kfree(tmp);

    }

    // release windows created by process
    winsys_remove_windows_by_pid(proc->id);

    removeProcessFromList(proc->id);

    //printf_term(proc->term,"\nKernel Process %d terminated.\n",proc->id);

    enable_scheduling();

    schedule();// force task switch.

}

int createProcess(char* exec, char *args) {

    pdirectory *prevDir = vmmngr_get_directory();

    char *k_args = kmalloc(strlen(args)+1);
    strcpy(k_args,args);

    char *k_exec = kmalloc(strlen(exec)+1);
    strcpy(k_exec,exec);

    /* get process virtual address space */
    pdirectory *addressSpace = create_address_space(); // *** REMEMBER TO FREE IT ONCE THE PROCESS TERMINATES
    void *running_proc_stack = (void *)get_current_task()->kernelESP-PAGE_SIZE;
    vmmngr_mmap_virt2virt(prevDir,addressSpace,running_proc_stack,running_proc_stack,I86_PDE_WRITABLE,I86_PTE_WRITABLE);

    disable_scheduling();
    vmmngr_switch_pdirectory(addressSpace); // this will only work when we're in a kernel process because were mapping kernel stacks to all processes, as opposed to user stacks which are individual.
    PImageInfo imageInfo = load_executable(addressSpace,k_exec);

    if (!imageInfo)
    {
        vmmngr_switch_pdirectory(prevDir);
        clear_kernel_space(addressSpace);
        vmmngr_free_pdir(addressSpace);
        enable_scheduling();
        return 0;
    }

    /* create PCB */

    process *proc = (process *)kcalloc(sizeof(process));

    proc->id            = getFreeID();
    proc->pageDirectory = addressSpace;
    proc->priority      = PRIORITY_MID;
    proc->state         = PROCESS_STATE_ACTIVE;
    proc->next = 0;
    proc->imageBase = imageInfo->ImageBase;
    proc->imageSize = imageInfo->ImageSize;
    proc->brk = imageInfo->ImageBase+imageInfo->ImageSize;
    proc->file_descs = 0;
    proc->name = kmalloc(strlen(k_exec)+1);
    strcpy(proc->name,k_exec);
    proc->threadList = 0;
    proc->livingThreads = 0;
    proc->is_kernel = false;

    /* Create userspace stack (4k size) */
    // void* stack = (void*) (imageInfo->ImageBase + imageInfo->ImageSize + PAGE_SIZE);

    // if ((uint32_t)stack % PAGE_SIZE != 0)
    //     stack += PAGE_SIZE - (uint32_t)stack % PAGE_SIZE;

    // /* map user process stack space */
    // vmmngr_alloc_virt (addressSpace, stack - PAGE_SIZE, // stack grows downwards
    // I86_PDE_WRITABLE|I86_PDE_USER,
    // I86_PTE_WRITABLE|I86_PTE_USER);

    void *stack = create_user_stack(addressSpace);
    void *esp = insert_argv_to_process_stack(k_args, stack+PAGE_SIZE);

    /* create thread descriptor */
    thread *mainThread       = (thread *)kcalloc(sizeof(thread));
    thread_create(mainThread,(void *)(imageInfo->EntryPointRVA + imageInfo->ImageBase),esp, false);
    vmmngr_switch_pdirectory(prevDir);
    enable_scheduling();
    vmmngr_unmap_virt(addressSpace,running_proc_stack);

    proc->term = get_running_process()->term;

    mainThread->parent = proc;
    mainThread->initialStack = stack+PAGE_SIZE;
    mainThread->isMain = true;
    mainThread->priority = proc->priority;

    kfree(imageInfo);
    kfree(k_exec);
    kfree(k_args);

    insert_process(proc);

    queue_insert(*mainThread);

    insert_thread_to_proc(proc,mainThread);

    return proc->id;
}

uintptr_t inc_proc_brk(uintptr_t inc)
{

    process *proc = get_running_process();

    if (proc == 0)
        return 0;

    if (proc->brk == 0)
        return 0;

    pdirectory *pdir = vmmngr_get_directory();

    uintptr_t old_brk = proc->brk;

    if (old_brk+inc > proc->imageBase+proc->imageSize+SBRK_LIM)
        return 0;

    proc->brk = old_brk+inc;

    if (old_brk%PAGE_SIZE + inc >= PAGE_SIZE || old_brk%PAGE_SIZE == 0)
    {
        uintptr_t page_aligned = old_brk;
        page_aligned -= page_aligned%PAGE_SIZE;

        while(page_aligned < proc->brk)
        {

            vmmngr_alloc_virt(pdir, (void *)page_aligned, I86_PDE_WRITABLE|I86_PDE_USER,I86_PTE_WRITABLE|I86_PTE_USER);

            page_aligned += PAGE_SIZE;

        }

    }

    return old_brk;

}

PFILE get_file_by_fd(process *proc, uint32_t fd)
{

    if (!proc)
        return 0;

    PFILE_DESC tmp = proc->file_descs;

    while(tmp)
    {

        if (tmp->fd == fd)
            return &tmp->file;

        tmp = tmp->next;

    }

    return 0;

}

uint32_t append_fd_to_process(process *proc, FILE f)
{

    if (!proc)
        return 0;

    PFILE_DESC container = (PFILE_DESC)kcalloc(sizeof(FILE_DESC));

    container->file = f;
    container->fd = 1;
    container->next = 0;

    if (!proc->file_descs)
    {
        proc->file_descs = container;
        return container->fd;
    }

    PFILE_DESC tmp = proc->file_descs;
    PFILE_DESC prev = 0;

    while (tmp)
    {
        if (tmp->fd >= container->fd)
                container->fd=tmp->fd+1;

        prev = tmp;
        tmp = tmp->next;

    }

    prev->next = container;

    return container->fd;

}

void close_fd(process *proc, uint32_t fd)
{

    if (!proc)
        return;

    PFILE_DESC tmp = proc->file_descs;

    if (tmp->next == 0)
    {

        if (tmp->fd == fd)
        {
            volCloseFile(&tmp->file);
            kfree(tmp);
            proc->file_descs = 0;
        }
        return;

    }

    while(tmp->next)
    {

        if (tmp->next->fd == fd)
        {

            PFILE_DESC nxt = tmp->next->next;

            volCloseFile(&tmp->next->file);
            kfree(tmp->next);

            tmp->next = nxt;

            return;

        }

        tmp = tmp->next;

    }

}

uint32_t fopen(char *path)
{

    process *proc = get_running_process();

    if (!proc)
        return 0;

    FILE f = volOpenFile(path);

    if (f.flags != FS_FILE)
        return 0;

    return append_fd_to_process(proc,f);

}

void fread(uint32_t fd, unsigned char* Buffer, unsigned int Length)
{

    volReadFile(get_file_by_fd(get_running_process(), fd), Buffer, Length);

}

void fclose(uint32_t fd)
{

    close_fd(get_running_process(),fd);

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
        while (tmp->next)
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

    proc->livingThreads++;

}

void waitForProcessToFinish(int pid)
{

    while(getProcessByID(pid))
        thread_sleep(500);

}

void terminateProcess()
{

    process *proc = get_running_process();

    if (!proc)
            return;
    if (proc->id==PROC_INVALID_ID)
            return;

    if (proc->is_kernel)
        terminateKernelProcessById(proc->id);
    else
        terminateProcessById(proc->id);

}

void terminateProcessById (int pid) {

    process *proc = getProcessByID(pid);

    if (!proc)
        return;

    disable_scheduling();

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

    /* unmap and release sbrked memory */
    for (uint32_t virt = proc->imageBase+fileSize; virt < proc->imageBase+fileSize+SBRK_LIM; virt+=4096) {

        /* unmap and release page */
        vmmngr_free_virt (proc->pageDirectory, (void *)virt);

    }

    // release all fds

    PFILE_DESC tmp_fd = proc->file_descs;

    while (tmp_fd)
    {

        PFILE_DESC tmp = tmp_fd->next;

        volCloseFile(&tmp_fd->file);
        kfree(tmp_fd);

        tmp_fd = tmp;

    }

    while (pThread)
    {

        // unmap virtual stack
        vmmngr_free_virt (proc->pageDirectory, (void *) pThread->initialStack-PAGE_SIZE); // stack is 4k
        thread *tmp = pThread;
        pThread = pThread->next;
        remove_by_tid(tmp->tid); // remove thread from thread list
        kfree(tmp);

    }

    // release windows created by process
    winsys_remove_windows_by_pid(proc->id);

    removeProcessFromList(proc->id);

    // printf_term(proc->term,"\nProcess %d terminated.\n",proc->id);

    enable_scheduling();

    schedule();// force task switch.

}

/* clones kernel space into new address space. */
void clone_kernel_space(pdirectory* out) {

    /* get current process directory. */
    pdirectory* proc = vmmngr_get_directory();

    /* copy kernel page tables into this new page directory.
    Recall that KERNEL SPACE is 0xc0000000, which starts at
    entry 768. */
    memcpy((char *)&out->m_entries[768], (char *)&proc->m_entries[768], 4 * sizeof (pd_entry));
    
    // also copy first 4mb
    memcpy((char *)&out->m_entries[0], (char *)&proc->m_entries[0], 1 * sizeof (pd_entry));
}

void clone_kernel_stacks(pdirectory *out)
{

    /* get current process directory. */
    pdirectory* proc = vmmngr_get_directory();

    memcpy((char *)&out->m_entries[896], (char *)&proc->m_entries[896], 128 * sizeof (pd_entry));

}

void clear_kernel_space(pdirectory *out)
{

    memset((char *)&out->m_entries[768], 0, 4 * sizeof (pd_entry));
    
    // also clear first 4mb
    memset((char *)&out->m_entries[0], 0, 1 * sizeof (pd_entry));

}

void clear_kernel_stacks(pdirectory *out)
{

    memset((char *)&out->m_entries[896], 0, 128 * sizeof (pd_entry));

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

        char *name = tmp->name;
        if (!name)
            name = "NAN";
        printf("name: %s, pid: %d\n",name, tmp->id);

        thread *tmp_thread = tmp->threadList;

        while (tmp_thread)
        {

            thread t = get_thread_by_tid(tmp_thread->tid);

            printf("tid: %d, state: ",t.tid);

            if(t.state & THREAD_TERMINATE)
                printf("TERMINATE\n");
            else if (t.state & THREAD_SUSPENDED)
                printf("SUSPENDED\n");
            else if (t.state & THREAD_BLOCK_SLEEP)
                printf("SLEEPING\n");
            else if (t.state & THREAD_RUN)
                printf("RUNNING\n");

            tmp_thread = tmp_thread->next;

        }

        tmp = tmp->next;

    }

}

bool does_process_own_term(process *p)
{

    terminal term = p->term;

    return term.term_win && term.term_inp_info;

}

void printf(char *fmt,...)
{

    va_list valist;
    va_start(valist,fmt);

    process *proc = get_running_process();

    if (!proc)
    {
        screen_vprintf(fmt,valist);
        return;
    }

    terminal term = proc->term;

    if (term.term_win && term.term_inp_info)
    {
        int pre_y = gfx_get_win_y(term.term_inp_info->cursor_offset_y);
        gfx_vprintf(term.term_win, term.term_inp_info, fmt,valist);
        int post_y = gfx_get_win_y(term.term_inp_info->cursor_offset_y);
        if (!term.term_inp_info->did_scroll)
            winsys_display_window_section(term.term_win,0,min(pre_y,post_y),term.term_win->width,abs(post_y-pre_y)+CHAR_HEIGHT);
        else
        {
            winsys_display_window(term.term_win);
            term.term_inp_info->did_scroll = false;
        }
    }
    else
        screen_vprintf(fmt,valist);

}

void printf_term(terminal term, char *fmt,...)
{

    va_list valist;
    va_start(valist,fmt);

    if (term.term_win && term.term_inp_info)
    {
        int pre_y = gfx_get_win_y(term.term_inp_info->cursor_offset_y);
        gfx_vprintf(term.term_win, term.term_inp_info, fmt,valist);
        int post_y = gfx_get_win_y(term.term_inp_info->cursor_offset_y);
        if (!term.term_inp_info->did_scroll)
            winsys_display_window_section(term.term_win,0,min(pre_y,post_y),term.term_win->width,abs(post_y-pre_y)+CHAR_HEIGHT);
        else
        {
            winsys_display_window(term.term_win);
            term.term_inp_info->did_scroll = false;
        }
    }

}

void putchar(char c)
{

    process *proc = get_running_process();

    if (!proc)
    {
        screen_putchar(c);
        return;
    }

    terminal term = proc->term;

    if (term.term_win && term.term_inp_info)
    {
        int pre_x = gfx_get_win_x(term.term_inp_info->cursor_offset_x)-CHAR_WIDTH;
        int pre_y = gfx_get_win_y(term.term_inp_info->cursor_offset_y);
        gfx_putchar(term.term_win, term.term_inp_info, c);
        winsys_display_window_section(term.term_win,pre_x,pre_y,CHAR_WIDTH*3,CHAR_HEIGHT);
    }
    else
        screen_putchar(c);

}

void print(char *msg)
{

    process *proc = get_running_process();

    if (!proc)
    {
        screen_print(msg);
        return;
    }

    terminal term = proc->term;

    if (term.term_win && term.term_inp_info)
    {
        int pre_y = gfx_get_win_y(term.term_inp_info->cursor_offset_y);
        gfx_print(term.term_win, term.term_inp_info, msg);
        int post_y = gfx_get_win_y(term.term_inp_info->cursor_offset_y);
        if (!term.term_inp_info->did_scroll)
            winsys_display_window_section(term.term_win,0,min(pre_y,post_y),term.term_win->width,abs(post_y-pre_y)+CHAR_HEIGHT);
        else
        {
            winsys_display_window(term.term_win);
            term.term_inp_info->did_scroll = false;
        }
    }
    else
        screen_print(msg);

}