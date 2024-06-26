#include "fiber.h"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ucontext.h>


#define FIBER_STACK_SIZE (4 << 12)

// Signal frame on stack.
struct sigframe {
    size_t r8, r9, r10, r11, r12, r13, r14, r15;
    size_t rdi, /*0x48*/rsi, rbp, rbx, rdx, rax, rcx, rsp;
    size_t /*0x80*/rip, flags;
};

struct Ctx
{
    struct ucontext_t ctx;

    struct Ctx* next;
    struct Ctx* prev;
    int deleted;
    int num;
    int isMain;
};

struct Delete
{
    void* info;
    struct Delete* next;
};
volatile sig_atomic_t started = 1;
struct Ctx* cur = NULL;
struct Delete* toDelete = NULL;

int fiberCnt = 0;

int C = 0;

struct Delete* MakeDeleter(){
    struct Delete* del = (struct Delete*)malloc(sizeof(struct Delete));
    del->next = NULL;
    return del;
}

struct Ctx* Alloc(){
    struct Ctx* node = (struct Ctx*)malloc(sizeof(struct Ctx));
    node->deleted = 0;
    node->isMain = 0;
    node->num = C++;
    fiberCnt++;
    return node;
}

void Free(struct Ctx* node){
    node->next->prev = node->prev;
    node->prev->next = node->next;
    free(node);
    fiberCnt--;
}

void Delete(){
    while (toDelete!=NULL){
        free(toDelete->info);
        struct Delete* tmp = toDelete;
        toDelete = toDelete->next;
        free(tmp);
    }
}

void Handler(void* data, void (*func)(void*)){
    func(data);
    cur->deleted = 1;
    FiberYield();
}
void* stack;
void FiberSpawn(void (*func)(void*), void* data){
    struct Ctx* newNode = Alloc();
    if (cur==NULL){
        cur = Alloc();
        cur->isMain = 1;
        cur->next = newNode;
        newNode->next = cur;
        cur->prev = newNode;
        newNode->prev = cur;
    }else{

        cur->prev->next = newNode;
        newNode->next = cur;
        newNode->prev = cur->prev;
        cur->prev = newNode;
    }
    getcontext(&newNode->ctx);
    newNode->ctx.uc_stack.ss_sp = malloc(FIBER_STACK_SIZE);
    newNode->ctx.uc_stack.ss_size = FIBER_STACK_SIZE;
    struct Delete* prev = toDelete;
    toDelete = MakeDeleter();
    toDelete->info = newNode->ctx.uc_stack.ss_sp;
    toDelete->next = prev;
    makecontext(&newNode->ctx,(void*)Handler,2,data,func);
    started = 1;
}

void FiberYield(){
    raise(SIGALRM);
}

int FiberTryJoin(){
    if (fiberCnt==1){
        started = 0;
        Free(cur);
        Delete();
        cur = NULL;
        return 1;
    }else{
        return 0;
    }
}

void fiber_sched(int signum, siginfo_t *info, void *context) {
    if (signum!=SIGALRM){
        return;
    }
    if (started==1){
        struct Ctx* old = cur;
        cur = cur->next;
        while (cur->deleted==1){
            cur = cur->next;
        }
        if (old->deleted==0){
            swapcontext(&old->ctx, &cur->ctx);
        }else{
            Free(old);
            setcontext(&cur->ctx);
        }
    }
}
struct sigaction sa;

void FiberInit() {
    // Need to disable stdlib buffer to use printf from fibers
    setbuf(stdout, NULL);
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_sigaction = fiber_sched;
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    ualarm(1000, 1000);
}


