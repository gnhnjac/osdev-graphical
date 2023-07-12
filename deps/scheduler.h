#pragma once
#include "process.h"

#define THREAD_RUN          1
#define THREAD_BLOCK_SLEEP  2
#define THREAD_TERMINATE    4

#define PRIORITY_HIGH 1
#define PRIORITY_MID 2
#define PRIORITY_LOW 3

typedef struct _queueEntry{

	thread thread;

	struct _queueEntry *next;

} queueEntry;

//refs
process *get_running_process();
void disable_scheduling();
void enable_scheduling();
void schedule();
void thread_set_state(thread* t, uint32_t flags);
void thread_remove_state(thread* t, uint32_t flags);
void thread_sleep(uint32_t ms);
void thread_wake();
void clear_queue();
bool queue_insert(thread t);
bool queue_insert_prioritized(thread t);
queueEntry *queue_remove();
thread *queue_get();
thread *queue_get_last();
void queue_delete_last();
void queue_delete_first();
thread get_thread_by_tid(int tid);
void remove_by_tid(int tid);
int get_free_tid();
void scheduler_dispatch ();
void scheduler_tick(void);
void scheduler_initialize(void);
void print_threads();
void idle_task();
void color_thread();
void test_thread();
void test_thread2();
void execute_idle();
void thread_execute(thread t);
void* create_kernel_stack();
void *create_user_kernel_stack();
void  thread_create (thread *t, void *entry, void *esp, bool is_kernel);
void fork();