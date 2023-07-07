#pragma once
#include "process.h"

#define THREAD_RUN          1
#define THREAD_BLOCK_SLEEP  2
#define THREAD_TERMINATE    4

typedef struct _queueEntry{

	thread thread;

	struct _queueEntry *next;

} queueEntry;

//refs
void disable_scheduling();
void enable_scheduling();
void schedule();
void thread_set_state(thread* t, uint32_t flags);
void thread_remove_state(thread* t, uint32_t flags);
void thread_sleep(uint32_t ms);
void thread_wake();
void clear_queue();
bool queue_insert(thread t);
thread *queue_remove();
thread *queue_get();
thread *queue_get_last();
void remove_by_tid(int tid);
int get_free_tid();
void scheduler_dispatch ();
void scheduler_tick(void);
void scheduler_initialize(void);
void print_threads();
void idle_task();
void test_thread();
void test_thread2();
void execute_idle();
void thread_execute(thread t);
void* create_kernel_stack();
void  thread_create (thread *t, void *entry, void *esp, bool is_kernel);