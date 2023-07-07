#include "scheduler.h"
#include "heap.h"
#include "memory.h"
#include "idt.h"
#include "vmm.h"
#include "pmm.h"
#include "shell.h"
#include "timer.h"
#include "screen.h"

queueEntry  *_readyQueue;
thread   _idleThread;
thread*  _currentTask = 0;

process *kernel_proc;

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

        /* force a task switch. */
        __asm__ ("int $32");
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
bool queue_insert(thread t) {
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

void remove_by_tid(int tid)
{

        queueEntry *tmp = _readyQueue;
        queueEntry *prev = 0;

        while (tmp)
        {

                if (tmp->thread.tid == tid)
                {
                        if (prev)
                                prev->next = tmp->next;
                        else
                                _readyQueue = tmp->next;

                        if (_currentTask->tid == tid)
                                _currentTask->state = THREAD_TERMINATE;

                        return;
                }

                prev = tmp;
                tmp = tmp->next;

        }

}

int get_free_tid()
{

        int id = 1;

        queueEntry *tmp = _readyQueue;

        while (tmp)
        {

                if (tmp->thread.tid == id)
                    id++;

                tmp = tmp->next;

        }

        return id;

}

/* schedule next task. */
void scheduler_dispatch () {

        /* We do Round Robin here, just remove and insert.
        Note _currentTask pointer always points to
        _currentThreadLocal. So just update _currentThreadLocal. */
        do
        {       
                queueEntry *tmp = queue_remove();
                queue_insert(tmp->thread);
                kfree(tmp);
                _currentTask = queue_get();

                /* adjust time delta. */
                if (_currentTask->sleepTimeDelta > 0)
                {
                        _currentTask->sleepTimeDelta--;

                        /* should we wake thread? */
                        if (_currentTask->sleepTimeDelta == 0) {
                                thread_wake();
                                return;
                        }

                }

        } while (_currentTask->state & THREAD_BLOCK_SLEEP);
}

void scheduler_tick(void)
{

        thread prev_task = *_currentTask;

        // dispatch the next thread
        scheduler_dispatch();

        // switch to it's address space if it's parent is different than the old parent
        if (prev_task.parent != _currentTask->parent || vmmngr_get_directory() != _currentTask->parent->pageDirectory)
                vmmngr_switch_pdirectory(_currentTask->parent->pageDirectory);

        if (prev_task.state == THREAD_TERMINATE)
        {
                if (prev_task.kernelESP != 0)
                        vmmngr_free_virt(prev_task.parent->pageDirectory, (void *)(prev_task.kernelESP-PAGE_SIZE));
                pmmngr_free_block(prev_task.parent->pageDirectory);
                kfree(prev_task.parent);
        }
}

extern void scheduler_isr(void);

/* initialize scheduler. */
void scheduler_initialize(void) {

        /* clear ready queue. */
        clear_queue();

        kernel_proc = (process *)kmalloc(sizeof(process));

        kernel_proc->id            = getFreeID();
        kernel_proc->pageDirectory = vmmngr_get_directory();
        kernel_proc->priority      = 1;
        kernel_proc->state         = PROCESS_STATE_ACTIVE;
        kernel_proc->next = 0;
        kernel_proc->threadList = 0;
        kernel_proc->name = (char *)kmalloc(6 + 1);
        strcpy(kernel_proc->name,"kernel");

        /* create idle thread and add it. */
        thread_create(&_idleThread, idle_task, create_kernel_stack(), true);

        _idleThread.parent = kernel_proc;

        queue_insert(_idleThread);

        insert_thread_to_proc(kernel_proc,queue_get_last());

        insert_process(kernel_proc);

        /* register isr */
        idt_set_gate(32, (void *)scheduler_isr, 0x8E|0x60);

}

void print_threads()
{

        queueEntry *tmp = _readyQueue;

        while (tmp)
        {

                printf("tid: %d, state: %b\n",tmp->thread.tid, tmp->thread.state);

                tmp = tmp->next;

        }

}

/* idle task. */
void idle_task() {

  enable_scheduling();

  /* setup other things since this is the first task called */

  /* create shell thread and add it. */
  thread shellThread;
  thread_create(&shellThread, shell_main, create_kernel_stack(), true);
  shellThread.parent = kernel_proc;

  queue_insert(shellThread);

  insert_thread_to_proc(kernel_proc,queue_get_last());

  // thread test1;
  // thread_create(&test1, test_thread, create_kernel_stack(), true);
  // queue_insert(test1);

  // thread test2;
  // thread_create(&test2, test_thread2, create_kernel_stack(), true);
  // queue_insert(test2);

  while(1) __asm__ ("pause");
}

void test_thread()
{
        char asc = 0;
        while (1)
        {

                putchar(asc+'a');

                wait_milliseconds(100);

                asc = (asc + 1) % 26;

        }

}

void test_thread2()
{
        char asc = 0;
        while (1)
        {

                putchar(asc+'0');

                wait_milliseconds(100);

                asc = (asc + 1) % 10;

        }

}

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

/* creates thread. */
void  thread_create (thread *t, void *entry, void *esp, bool is_kernel) {

	trapFrame* frame;

	/* kernel and user selectors. */
#define USER_DATA   0x23
#define USER_CODE   0x1b
#define KERNEL_DATA 0x10
#define KERNEL_CODE 8

	/* adjust stack. We are about to push data on it. */
	esp -= sizeof (trapFrame);

	/* initialize task frame. */
	frame = ((trapFrame*) esp);
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

	/* set up segment selectors. */
        if (is_kernel)
        {
        	frame->cs    = KERNEL_CODE;
        	frame->ds    = KERNEL_DATA;
        	frame->es    = KERNEL_DATA;
        	frame->fs    = KERNEL_DATA;
        	frame->gs    = KERNEL_DATA;
        	t->SS         = KERNEL_DATA;
                t->kernelESP = 0;
                t->kernelSS = 0;
        }
        else
        {
                frame->cs    = USER_CODE;
                frame->ds    = USER_DATA;
                frame->es    = USER_DATA;
                frame->fs    = USER_DATA;
                frame->gs    = USER_DATA;
                t->SS         = USER_DATA;
                t->kernelESP = (uint32_t)create_kernel_stack();
                t->kernelSS = KERNEL_DATA;
        }


        static int cntr = 0;

        cntr++;

        if (cntr == 3)
        {
                printf("%U",esp);
                while(1);
        }

 
	/* set stack. */
	t->ESP = (uint32_t)esp;

	t->parent   = 0;
	t->priority = 0;
	t->state    = THREAD_RUN;
	t->sleepTimeDelta = 0;
        t->tid = get_free_tid();
        t->next = 0;
}