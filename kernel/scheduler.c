#include "scheduler.h"
#include "heap.h"
#include "memory.h"
#include "idt.h"
#include "vmm.h"
#include "pmm.h"
#include "shell.h"
#include "timer.h"
#include "screen.h"
#include "graphics.h"
#include "strings.h"

queueEntry  *_readyQueue;
thread   _idleThread;
thread*  _currentTask = 0;

process *kernel_proc;

process *get_running_process()
{

        if (_currentTask)
                return _currentTask->parent;
        return 0;

}
thread *get_current_task()
{

        return _currentTask;

}

void disable_scheduling()
{
        _currentTask = 0;
}

void enable_scheduling()
{
        _currentTask = queue_get();
}

/* schedule new task to run. */
void schedule() {

        /* force a task switch without invoking the pit */
        __asm__("int $0x81");

}

/* set thread state flags. */
void thread_set_state(thread* t, uint32_t flags) {

        /* set flags. */
        t->state |= flags;
}

/* remove thread state flags. */
void thread_remove_state(thread* t, uint32_t flags) {

        /* remove flags. */
        t->state &= ~flags;
}

void thread_set_state_by_id(int tid, uint32_t state)
{

        bool scheduling_enabled = (_currentTask != 0);

        if (scheduling_enabled)
                disable_scheduling();

        queueEntry *tmp = _readyQueue;

        while (tmp)
        {
                if (tmp->thread.tid == tid)
                {       
                        thread_set_state(&tmp->thread, state);
                        if (scheduling_enabled)
                                enable_scheduling();
                        return;
                }

                tmp = tmp->next;

        }

        if (scheduling_enabled)
                enable_scheduling();

}

void thread_sleep(uint32_t ms) {

        /* go to sleep. */
        thread_set_state(_currentTask,THREAD_BLOCK_SLEEP);
        _currentTask->sleepTimeDelta = ms/(1000/PHASE);
        schedule();
}

void thread_wake() {

        /* wake up. */
        thread_remove_state(_currentTask,THREAD_BLOCK_SLEEP);
        _currentTask->sleepTimeDelta = 0;
}

/* clear queue. */
void clear_queue() {
        queueEntry *queue = _readyQueue;
        while (queue)
        {
                queueEntry *tmp = queue;

                queue = queue->next;

                kfree(tmp);
        }

        _readyQueue = 0;
}

/* insert thread. */
void queue_insert(thread t) {

        bool scheduling_enabled = (_currentTask != 0);

        if (scheduling_enabled)
                disable_scheduling();

        queueEntry *tmp = _readyQueue;

        queueEntry *new = (queueEntry *)kmalloc(sizeof(queueEntry));
        new->next = 0;
        //memcpy(&new->thread,&t,sizeof(thread));
        new->thread = t;
        if (tmp)
        {
                while (tmp->next)
                        tmp = tmp->next;
                tmp->next = new;
        }
        else
        {
                _readyQueue = new;
        }

        if (scheduling_enabled)
                enable_scheduling();

}

/* insert thread by priority. */
void queue_insert_prioritized(thread t) {

        //bool scheduling_enabled = (_currentTask != 0);

        //if (scheduling_enabled)
        //        disable_scheduling();

        queueEntry *tmp = _readyQueue;

        queueEntry *new = (queueEntry *)kmalloc(sizeof(queueEntry));
        new->next = 0;
        //memcpy(&new->thread,&t,sizeof(thread));
        new->thread = t;
        if (tmp)
        {
                while (tmp->next && tmp->thread.priority <= t.priority)
                        tmp = tmp->next;
                new->next = tmp->next;
                tmp->next = new;
        }
        else
        {
                _readyQueue = new;
        }

        //if (scheduling_enabled)
        //        enable_scheduling();

}

/* insert queue entry by priority. */
void queue_insert_prioritized_queueEntry(queueEntry *new) {

        //bool scheduling_enabled = (_currentTask != 0);

        //if (scheduling_enabled)
        //        disable_scheduling();

        queueEntry *tmp = _readyQueue;

        new->next = 0;
        //memcpy(&new->thread,&t,sizeof(thread));
        if (tmp)
        {
                while (tmp->next && tmp->thread.priority <= new->thread.priority)
                        tmp = tmp->next;
                new->next = tmp->next;
                tmp->next = new;
        }
        else
        {
                _readyQueue = new;
        }

        //if (scheduling_enabled)
        //        enable_scheduling();

}

/* remove thread. */
queueEntry *queue_remove() {
        queueEntry *t;
        if (_readyQueue)
        {
                t = _readyQueue;

                _readyQueue = _readyQueue->next;

        }
        return t;
}

/* get top of queue. */
thread *queue_get() {
        if (_readyQueue)
                return &_readyQueue->thread;
        return 0;
}

/* get bot of queue. */
thread *queue_get_last() {

        queueEntry *tmp = _readyQueue;
        if (tmp)
        {
                while (tmp->next)
                        tmp = tmp->next;
                return &tmp->thread;
        }
        else
                return 0;
}

void queue_delete_last()
{

        queueEntry *tmp = _readyQueue;
        if (!tmp)
                return;

        if (!tmp->next)
        {
                kfree(tmp);
                _readyQueue = 0;
        }
        while(tmp->next->next)
                tmp = tmp->next;

        kfree(tmp->next);
        tmp->next = 0;

}

void queue_delete_first()
{

        if (_readyQueue)
        {

                queueEntry *tmp = _readyQueue;
                _readyQueue = _readyQueue->next;
                kfree(tmp);

        }

}

thread get_thread_by_tid(int tid)
{

        bool scheduling_enabled = (_currentTask != 0);

        if (scheduling_enabled)
                disable_scheduling();

        thread t;
        t.tid = -1;

        queueEntry *tmp = _readyQueue;

        while (tmp)
        {
                if (tmp->thread.tid == tid)
                {       
                        t = tmp->thread;
                        if (scheduling_enabled)
                                enable_scheduling();
                        return t;
                }

                tmp = tmp->next;

        }

        if (scheduling_enabled)
                enable_scheduling();

        return t;

}

void remove_by_tid(int tid)
{

        thread_set_state_by_id(tid,THREAD_TERMINATE);

}

int get_free_tid()
{

        int id = 1;

        queueEntry *tmp = _readyQueue;

        while (tmp)
        {

                if (tmp->thread.tid >= id)
                    id=tmp->thread.tid+1;

                tmp = tmp->next;

        }

        return id;

}

/* schedule next task. */
void scheduler_dispatch () {

        bool is_terminate;

        /* We do Round Robin here, just remove and insert.*/
        do
        {       
                queueEntry *tmp = queue_remove();
                queue_insert_prioritized_queueEntry(tmp);
                _currentTask = queue_get();

                is_terminate = false;

                if (_currentTask->state & THREAD_TERMINATE)
                {

                        if (_currentTask->kernelESP != 0)
                        {
                                vmmngr_free_virt(_currentTask->parent->pageDirectory, (void *)(_currentTask->kernelESP-PAGE_SIZE));
                                _currentTask->parent->livingThreads--;
                        }
                        if (_currentTask->parent->livingThreads == 0) // bad because main might execute before others thus the prev line will throw an error
                        {
                                clear_kernel_space(_currentTask->parent->pageDirectory);
                                clear_kernel_stacks(_currentTask->parent->pageDirectory);
                                vmmngr_free_pdir(_currentTask->parent->pageDirectory);
                                kfree(_currentTask->parent);
                        }
                        queue_delete_first();
                        is_terminate = true;
                        continue;

                }

        } while (_currentTask->state & THREAD_BLOCK_SLEEP || is_terminate);

        // adjust time delta (this is kinda faulty because when you call schedule it isn't necessarily on a PHASE irq call, it can be int 0x81)
        queueEntry *tmp = _readyQueue;

        while(tmp)
        {

                if (tmp->thread.sleepTimeDelta > 0)
                {
                        tmp->thread.sleepTimeDelta--;
                        /* should we wake thread? */
                        if (tmp->thread.sleepTimeDelta == 0)
                        {
                                thread_remove_state(&tmp->thread,THREAD_BLOCK_SLEEP);
                                tmp->thread.sleepTimeDelta = 0;
                        }
                }
                tmp = tmp->next;

        }

        static uint32_t idle_task_ticks = 0;
        static uint32_t total_ticks = 0;

        if (_currentTask->tid == 1)
                idle_task_ticks++;
        total_ticks++;

        if (total_ticks == PHASE)
        {
                char cpu_percentage_str[3 + 1];
                int_to_str_padding(100-idle_task_ticks*100/total_ticks,cpu_percentage_str,10,2);
                cpu_percentage_str[2] = '%';
                cpu_percentage_str[3] = 0;

                int i = 0;
                while(i < 3)
                {
                    display_psf1_8x16_char_bg(cpu_percentage_str[i],get_screen_x(i+5),0,0xF,0);
                    i++;
                }

                idle_task_ticks = 0;
                total_ticks = 0;
        }
}

void scheduler_tick(void)
{

        //thread prev_task = *_currentTask;

        // dispatch the next thread
        scheduler_dispatch();

        // switch to it's address space if it's parent is different than the old parent
        // cant do that here because we're still in the old esp, maybe it's not virtually mapped in the new page directory
        // if (prev_task.parent != _currentTask->parent || vmmngr_get_directory() != _currentTask->parent->pageDirectory)
        //         vmmngr_switch_pdirectory(_currentTask->parent->pageDirectory);
}

extern void scheduler_isr(void);
extern void scheduler_raw(void);

/* initialize scheduler. */
void scheduler_initialize(void) {

        /* clear ready queue. */
        clear_queue();

        kernel_proc = (process *)kcalloc(sizeof(process));

        kernel_proc->id            = getFreeID();
        kernel_proc->pageDirectory = vmmngr_get_directory();
        kernel_proc->priority      = PRIORITY_HIGH;
        kernel_proc->state         = PROCESS_STATE_ACTIVE;
        kernel_proc->next = 0;
        kernel_proc->threadList = 0;
        kernel_proc->name = (char *)kmalloc(6 + 1);
        strcpy(kernel_proc->name,"kernel");
        insert_process(kernel_proc);

        /* create idle thread and add it. */
        thread_create(&_idleThread, idle_task, create_kernel_stack(), true);
        _idleThread.parent = kernel_proc;
        _idleThread.isMain = true;
        _idleThread.priority = PRIORITY_LOW;
        queue_insert(_idleThread);
        insert_thread_to_proc(kernel_proc,&_idleThread);

        /* create shell thread and add it. */
        thread *shellThread = (thread *)kmalloc(sizeof(thread));
        thread_create(shellThread, shell_main, create_kernel_stack(), true);
        shellThread->parent = kernel_proc;
        shellThread->isMain = false;
        shellThread->priority = PRIORITY_HIGH;
        queue_insert(*shellThread);
        insert_thread_to_proc(kernel_proc,shellThread);

        /* register isr */
        idt_set_gate(32, (void *)scheduler_isr, 0x8E|0x60);
        idt_set_gate(0x81, (void *)scheduler_raw, 0x8E|0x60); // present|32 bit interrupt gate|dpl 3

}

/* idle task. */
void idle_task() {

  enable_scheduling();

  /* setup other things since this is the first task called */

  thread *t = (thread *)kmalloc(sizeof(thread));
  thread_create(t, cycle_colors, create_kernel_stack(), true);
  t->parent = kernel_proc;
  t->isMain = false;
  t->priority = PRIORITY_LOW;
  queue_insert(*t);
  insert_thread_to_proc(kernel_proc,t);

  while(1) __asm__ ("pause");
}

void cycle_colors()
{

  int cycler = 0;
  while(1)
  {
          display_psf1_8x16_char_bg('a', get_screen_x(0),0, 0xf, (cycler+3)%15);
          display_psf1_8x16_char_bg('k', get_screen_x(1),0, 0xf, (cycler+2)%15);
          display_psf1_8x16_char_bg('o', get_screen_x(2),0, 0xf, (cycler+1)%15);
          display_psf1_8x16_char_bg('s', get_screen_x(3),0, 0xf, cycler);
          cycler = (cycler + 1) % 15;
          thread_sleep(200);
  } 

}

// #include "lock.h"

// lock_t lock = ATOMIC_LOCK_INIT;
// int asd = 0;
// void lock_test()
// {

//         for(int i = 0; i < 10; i++)
//         {
//                 acquireLock(&lock);
//                 int var = asd;
//                 var++;
//                 thread_sleep(10);
//                 asd = var;
//                 releaseLock(&lock);
//         }

//         printf("done my part, asd: %d\n",asd);

//         while(1);
// }

/* execute idle thread. */
void execute_idle() {

        /* just run idle thread. */
        thread_execute (_idleThread);
}

/* executes thread. */
void thread_execute(thread t) {
        __asm__(
                "mov %0, %%esp\n"
                "pop %%gs\n"
                "pop %%fs\n"
                "pop %%es\n"
                "pop %%ds\n"
                "popa\n"
                "iret" : : "m" (t.ESP)
        );
}

int _kernel_stack_index = 0;
/* create a new kernel space stack. */
void* create_kernel_stack() {

        /* we are reserving this area for 4k kernel stacks. */
#define KERNEL_STACK_ALLOC_BASE 0xe0000000

        uint32_t loc = KERNEL_STACK_ALLOC_BASE + _kernel_stack_index * PAGE_SIZE;

        vmmngr_alloc_virt(vmmngr_get_directory(), (void *)loc, I86_PDE_WRITABLE, I86_PTE_WRITABLE);

        /* we are returning top of stack. */
        void *ret = (void*) (loc + PAGE_SIZE);

        _kernel_stack_index++;

        /* and return top of stack. */
        return ret;
}

/* create a new kernel space stack for user mode thread. */
void *create_user_kernel_stack()
{

/* we are reserving this area for 4k kernel stacks. */
#define USER_KERNEL_STACK_ALLOC_BASE 0x80000000

        uint32_t loc = USER_KERNEL_STACK_ALLOC_BASE;

        while(vmmngr_check_virt_present(vmmngr_get_directory(), (void *)loc))
                loc += PAGE_SIZE;

        vmmngr_alloc_virt(vmmngr_get_directory(), (void *)loc, I86_PDE_WRITABLE, I86_PTE_WRITABLE);

        /* we are returning top of stack. */
        void *ret = (void*) (loc + PAGE_SIZE);

        /* and return top of stack. */
        return ret;

}

/* create a new user space stack for user mode thread. */
void *create_user_stack(pdirectory *addressSpace)
{

        uint32_t loc = USER_KERNEL_STACK_ALLOC_BASE-PAGE_SIZE; // we're growing downwards from the kernel stack alloc base

        while(vmmngr_check_virt_present(addressSpace, (void *)loc))
                loc -= PAGE_SIZE;

        vmmngr_alloc_virt(addressSpace, (void *)loc, I86_PDE_WRITABLE|I86_PDE_USER, I86_PTE_WRITABLE|I86_PTE_USER);

        /* and return bot of stack. */
        return (void*)loc;

}

/* allocate user space pages for user mode thread. */
void *allocate_user_space_pages(int page_amt)
{

/* we are reserving this area for 4k space chunks. */
#define USER_SPACE 0xA0000000

        void *loc;

        pdirectory *pdir = vmmngr_get_directory();

        int count = 0;
        int i = 0;
        while(1)
        {

                if (USER_SPACE+i*PAGE_SIZE >= 0xC0000000)
                        return 0;

                if (!vmmngr_check_virt_present(pdir, (void *)USER_SPACE+i*PAGE_SIZE))
                {
                        if (count == 0)
                                loc = (void *)((uint32_t)USER_SPACE+i*PAGE_SIZE);
                        count++;
                }
                else
                        count = 0;

                if (count == page_amt)
                        break;

                i++;
        }

        for (i = 0; i < page_amt; i++)
                vmmngr_alloc_virt(pdir, loc+i*PAGE_SIZE, I86_PDE_WRITABLE|I86_PDE_USER, I86_PTE_WRITABLE|I86_PTE_USER);

        return loc;

}

/* find free user space pages for user mode thread. */
void *find_user_space_pages(void *base, int page_amt)
{

        void *loc;

        pdirectory *pdir = vmmngr_get_directory();

        int count = 0;
        int i = 0;
        while(1)
        {

                if ((uint32_t)base+i*PAGE_SIZE >= 0xC0000000)
                        return 0;

                if (!vmmngr_check_virt_present(pdir, (void *)base+i*PAGE_SIZE))
                {
                        if (count == 0)
                                loc = (void *)((uint32_t)base+i*PAGE_SIZE);
                        count++;
                }
                else
                        count = 0;

                if (count == page_amt)
                        break;

                i++;
        }

        return loc;

}

/* creates thread. */
void  thread_create (thread *t, void *entry, void *esp, bool is_kernel) {

        /* kernel and user selectors. */
#define USER_DATA   0x23
#define USER_CODE   0x1b
#define KERNEL_DATA 0x10
#define KERNEL_CODE 8


        /* set up segment selectors. */
        if (is_kernel)
        {

                /* adjust stack. We are about to push data on it. */
                esp -= sizeof (trapFrame);

                /* initialize task frame. */
                trapFrame*frame = ((trapFrame*) esp);
                frame->flags = 0x202;
                frame->eip   = (uint32_t)entry;
                frame->ebp   = 0;
                frame->esp   = 0;
                frame->edi   = 0;
                frame->esi   = 0;
                frame->edx   = 0;
                frame->ecx   = 0;
                frame->ebx   = 0;
                frame->eax   = 0;

                frame->cs    = KERNEL_CODE;
                frame->ds    = KERNEL_DATA;
                frame->es    = KERNEL_DATA;
                frame->fs    = KERNEL_DATA;
                frame->gs    = KERNEL_DATA;
                t->SS        = KERNEL_DATA;
                t->kernelESP = 0;
                t->kernelSS = 0;

                /* set stack. */
                t->ESP = (uint32_t)esp;
        }
        else
        {

                void *kernel_esp = create_user_kernel_stack();
                t->kernelESP = (uint32_t)kernel_esp;
                t->kernelSS = KERNEL_DATA;

                /* adjust stack. We are about to push data on it. */
                kernel_esp -= sizeof (userTrapFrame);

                /* initialize task frame. */
                userTrapFrame* frame = (userTrapFrame*) kernel_esp;
                frame->flags = 0x202;
                frame->eip   = (uint32_t)entry;
                frame->ebp   = 0;
                frame->esp   = 0;
                frame->edi   = 0;
                frame->esi   = 0;
                frame->edx   = 0;
                frame->ecx   = 0;
                frame->ebx   = 0;
                frame->eax   = 0;

                frame->cs    = USER_CODE;
                frame->ds    = USER_DATA;
                frame->es    = USER_DATA;
                frame->fs    = USER_DATA;
                frame->gs    = USER_DATA;
                t->SS        = USER_DATA;
                frame->user_stack = (uint32_t)esp;
                frame->user_ss = USER_DATA;

                /* set stack. */
                t->ESP = (uint32_t)kernel_esp;
        }


        t->parent   = 0;
        t->priority = 0;
        t->state    = THREAD_RUN;
        t->sleepTimeDelta = 0;
        t->tid = get_free_tid();
        t->next = 0;
}

void *insert_argv_to_process_stack(char *args, void *esp)
{

        int arg_count;

        if (!args)
                arg_count = 0;
        else
                arg_count = count_substrings(args, ' '); // including cmd

        char **argv = (char **)kmalloc((arg_count+1)*sizeof(int)); // +1 for null terminator

        for (int i = 0; i < arg_count; i++)
        {
                char *arg = seperate_and_take(args, ' ', i);

                uint32_t int_aligned_len = strlen(arg)+1;

                if (int_aligned_len % 4 != 0)
                        int_aligned_len += 4 - int_aligned_len%4;

                esp -= int_aligned_len; // make place for the argument

                strcpy((char *)esp,arg);

                argv[i] = (char *)esp;

                kfree(arg);
        }

        argv[arg_count] = 0;

        esp -= (arg_count+1)*sizeof(int);

        memcpy((char *)esp,(char *)argv,(arg_count+1)*sizeof(int));

        esp -= sizeof(int);

        *(char ***)esp = (char **)(esp+sizeof(int));

        esp -= sizeof(int);

        *(int *)esp = arg_count;

        esp -= sizeof(int);

        kfree(argv);

        return esp;
}

extern uint32_t read_eip();

int fork()
{

        disable_scheduling();

        thread *parent_task = queue_get();

        /* create thread descriptor */
        thread *th = (thread *)kmalloc(sizeof(thread));

        /* Create userspace stack (4k size) */
        //void* stack = parent_task->initialStack;

        pdirectory *addressSpace = vmmngr_get_directory();

        void *stack = create_user_stack(addressSpace);

        // while(vmmngr_check_virt_present(addressSpace,stack))
        //         stack += PAGE_SIZE;

        // /* map user process stack space */
        // vmmngr_alloc_virt (addressSpace, stack,
        // I86_PDE_WRITABLE|I86_PDE_USER,
        // I86_PTE_WRITABLE|I86_PTE_USER);

        uint32_t stack_diff = stack-parent_task->initialStack+PAGE_SIZE;

        uint32_t *tmp_user_parent_stack = (uint32_t *)(parent_task->initialStack-PAGE_SIZE);
        uint32_t *tmp_user_child_stack = (uint32_t *)stack;

        uint32_t i = 0;
        while(i < PAGE_SIZE/4)
        {
                tmp_user_child_stack[i] = tmp_user_parent_stack[i];

                i += 1;

        }

        void *kernel_esp = create_user_kernel_stack();

        uint32_t kernel_stack_diff = ((uint32_t)kernel_esp)-parent_task->kernelESP;

        th->kernelESP = (uint32_t)kernel_esp;
        th->kernelSS = KERNEL_DATA;

        uint32_t *parent_kernel_esp;
        __asm__("mov %%esp, %0" : "=m" (parent_kernel_esp));

        kernel_esp -= (parent_task->kernelESP-(uint32_t)parent_kernel_esp);
        uint32_t *kernel_esp_ptr = (uint32_t *)kernel_esp;

        while(parent_kernel_esp < (uint32_t *)(parent_task->kernelESP-4*5))
        {

                *kernel_esp_ptr = *parent_kernel_esp;

                kernel_esp_ptr+=1;
                parent_kernel_esp+=1;

        }

        uint32_t parent_user_eip = parent_kernel_esp[0];
        uint32_t parent_user_code_segment = parent_kernel_esp[1];
        uint32_t parent_user_flags = parent_kernel_esp[2];
        uint32_t parent_user_stack_pointer = parent_kernel_esp[3];
        uint32_t parent_user_stack_segment = parent_kernel_esp[4];

        kernel_esp_ptr[0] = parent_user_eip;
        kernel_esp_ptr[1] = parent_user_code_segment;
        kernel_esp_ptr[2] = parent_user_flags;
        kernel_esp_ptr[3] = parent_user_stack_pointer+stack_diff;
        kernel_esp_ptr[4] = parent_user_stack_segment;

        /* adjust stack. We are about to push data on it. */
        kernel_esp -= sizeof (trapFrame);

        /* initialize task frame. */
        trapFrame* frame = (trapFrame*) kernel_esp;
        frame->flags = 0x202;

        uint32_t parent_kernel_ebp;
        __asm__("mov %%ebp, %0" : "=m" (parent_kernel_ebp));

        uint32_t *child_kernel_ebp = (uint32_t *)(parent_kernel_ebp+kernel_stack_diff);
        uint32_t *child_kernel_ebp_tmp = child_kernel_ebp;
        while (*child_kernel_ebp_tmp)
        {

                *child_kernel_ebp_tmp += stack_diff;
                child_kernel_ebp_tmp = (uint32_t *)*child_kernel_ebp_tmp;

        }
        frame->ebp   = (uint32_t)child_kernel_ebp;
        
        frame->esp   = 0;
        frame->edi   = 0;
        frame->esi   = 0;
        frame->edx   = 0;
        frame->ecx   = 0;
        frame->ebx   = 0;
        frame->eax   = 0;

        frame->cs    = KERNEL_CODE;
        frame->ds    = USER_DATA;
        frame->es    = USER_DATA;
        frame->fs    = USER_DATA;
        frame->gs    = USER_DATA;
        th->SS       = USER_DATA;

        /* set stack. */
        th->ESP = (uint32_t)kernel_esp;
        
        th->state    = THREAD_RUN;
        th->sleepTimeDelta = 0;
        uint32_t child_tid = get_free_tid();
        th->tid = child_tid;
        th->next = 0;
        th->parent = parent_task->parent;
        th->initialStack = stack+PAGE_SIZE;
        th->isMain = false;
        th->priority = parent_task->priority;

        insert_thread_to_proc(parent_task->parent,th);
        queue_insert(*th);

        frame->eip = read_eip();

        if (_currentTask) // if were parent this should never apply because we disabled scheduling
        {
                return 0;
        }
        else
        {

                enable_scheduling();
                return child_tid;
        }

}

// int pthread_create(thread *user_thread, void *start_routine, uint32_t arg)
// {

//         disable_scheduling();

//         thread *parent_task = queue_get();

//         /* create thread descriptor */
//         thread *th = (thread *)kmalloc(sizeof(thread));

//         /* Create userspace stack (4k size) */
//         //void* stack = parent_task->initialStack;

//         pdirectory *addressSpace = vmmngr_get_directory();

//         void *stack = create_user_stack(addressSpace);
//         *(stack-4) = arg;

//         // while(vmmngr_check_virt_present(addressSpace,stack))
//         //         stack += PAGE_SIZE;

//         // /* map user process stack space */
//         // vmmngr_alloc_virt (addressSpace, stack,
//         // I86_PDE_WRITABLE|I86_PDE_USER,
//         // I86_PTE_WRITABLE|I86_PTE_USER);

//         void *kernel_esp = create_user_kernel_stack();
//         t->kernelESP = (uint32_t)kernel_esp;
//         t->kernelSS = KERNEL_DATA;

//         /* adjust stack. We are about to push data on it. */
//         kernel_esp -= sizeof (userTrapFrame);

//         /* initialize task frame. */
//         userTrapFrame* frame = (userTrapFrame*) kernel_esp;
//         frame->flags = 0x202;
//         frame->eip   = (uint32_t)start_routine;
//         frame->ebp   = 0;
//         frame->esp   = 0;
//         frame->edi   = 0;
//         frame->esi   = 0;
//         frame->edx   = 0;
//         frame->ecx   = 0;
//         frame->ebx   = 0;
//         frame->eax   = 0;

//         frame->cs    = USER_CODE;
//         frame->ds    = USER_DATA;
//         frame->es    = USER_DATA;
//         frame->fs    = USER_DATA;
//         frame->gs    = USER_DATA;
//         t->SS        = USER_DATA;
//         frame->user_stack = (uint32_t)(stack-8);
//         frame->user_ss = USER_DATA;

//         /* set stack. */
//         t->ESP = (uint32_t)kernel_esp;
        
//         th->state    = THREAD_RUN;
//         th->sleepTimeDelta = 0;
//         uint32_t child_tid = get_free_tid();
//         th->tid = child_tid;
//         th->next = 0;
//         th->parent = parent_task->parent;
//         th->initialStack = stack+PAGE_SIZE;
//         th->isMain = false;
//         th->priority = parent_task->priority;

//         memcpy(user_thread,th,sizeof(thread));

//         insert_thread_to_proc(parent_task->parent,th);
//         queue_insert(*th);

//         enable_scheduling();
//         return child_tid;

// }