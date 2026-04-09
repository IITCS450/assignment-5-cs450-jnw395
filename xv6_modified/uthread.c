#include "types.h"
#include "stat.h"
#include "user.h"
#include "uthread.h"

// TODO: Implement cooperative user-level threads.
#define MAX_THREADS 8

// thread classifiers
#define FREE 0
#define RUNNABLE 1
#define RUNNING 2
#define ZOMBIE 3

// thread struct
typedef struct thread {
    tid_t tid;
    int state;
    void *stack;
    void *sp;  // stack pointer
} thread_t;

static thread_t threads[MAX_THREADS];
static thread_t *current_thread;

void thread_init(void){
    for(int i=0; i < MAX_THREADS; i++){
        threads[i].state = FREE;
    }

    // main thread = thread 0
    threads[0].tid = 0;
    threads[0].state = RUNNING;
    threads[0].stack = 0;   // use existing stack
    threads[0].sp = 0;

    current_thread = &threads[0];
}
// Declarations so thread_create() can use it
static void thread_stub(void (*fn)(void*), void *arg);
extern void uswtch(void **old_sp, void *new_sp);

tid_t thread_create(void (*fn)(void*), void *arg){
    for(int i=0; i < MAX_THREADS; i++){
        if(threads[i].state == FREE){

            thread_t *t = &threads[i];
            t -> tid = i;
            t -> state = RUNNABLE;

            // allocate stack
            t -> stack = malloc(4096);
            if(t->stack == 0){
                return -1;
            }

            // stack builds downwards
            void **sp = (void**)((char*) t-> stack + 4096);

            // build stack frame for thread_stub(fn, arg)
            *(--sp) = arg;            // arg 2
            *(--sp) = (void*)fn;      // arg 1
            *(--sp) = 0;              // fake return address for thread_stub
            *(--sp) = (void*)thread_stub; // ret in uswtch lands here
                        
            *(--sp) = 0; // ebp
            *(--sp) = 0; // ebx
            *(--sp) = 0; // esi
            *(--sp) = 0; // edi

            t -> sp = sp;

            return t->tid;
        }
    }
    return -1; 
}

static void thread_stub(void (*fn)(void*), void *arg){
    fn(arg);
    current_thread -> state = ZOMBIE;

    for(;;) thread_yield();
}

static thread_t* pick_next(){
    int start = current_thread->tid;

    for(int i=1; i < MAX_THREADS; i++){
        int idx = (start + i) % MAX_THREADS;

        if(threads[idx].state == RUNNABLE){
            return &threads[idx];
        }

    }
    
    return current_thread;
}

void thread_yield(void){
    thread_t *next = pick_next();
    if(next == current_thread){
        return;
    }
    thread_t *prev = current_thread;
    current_thread = next;
    if(prev->state != ZOMBIE)
        prev->state = RUNNABLE;
    next->state = RUNNING;
    uswtch(&prev->sp, next->sp);
}

int thread_join(tid_t tid){ 
    if(tid < 0 || tid >= MAX_THREADS){
        return -1;
    }

    while(threads[tid].state != ZOMBIE){
        thread_yield();
    }

    free(threads[tid].stack);
    threads[tid].state = FREE;

    return 0;
}
