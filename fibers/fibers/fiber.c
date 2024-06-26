#include "fiber.h"
#include <stdlib.h>
#include <stdio.h>
struct Ctx
{
    size_t rsp;

    size_t rbx;

    size_t r12;
    size_t r13;
    size_t r14;
    size_t r15;

    size_t rbp;

    size_t rdi;

    size_t rip;

    size_t rax;

    size_t rdx;

    void* stack;

    struct Ctx* next;
    struct Ctx* prev;
    int deleted;
    int started;
    int num;
};

int C = 0;

struct Ctx* Alloc(){
    struct Ctx* node = (struct Ctx*)malloc(sizeof(struct Ctx));
    node->deleted = 0;
    node->started = 0;
    node->num = C++;
    return node;
}

void Free(struct Ctx* node){
    printf("deleteing num = %d\n",node->num);
    if (node->stack!=NULL){
        free(node->stack);
    }
    node->next->prev = node->prev;
    node->prev->next = node->next;
    free(node);
}

struct Ctx* cur = NULL;

int fiberCnt = 0;

void Handler(void* data, void (*func)(void*)){
    func(data);
    cur->deleted = 1;
    printf("something is done i can not believe it\n");
    FiberYield();
}

void FiberSpawn(void (*func)(void*), void* data){
    struct Ctx* newNode = Alloc();
    if (cur==NULL){

        fiberCnt = 2;

        cur = Alloc();
        printf("mainNode = %ld\n",(size_t)cur);
        cur->started = 1;

        cur->next = newNode;
        newNode->next = cur;
        cur->prev = newNode;
        newNode->prev = cur;

        cur->stack = NULL; // can check if this Fiber is main

        printf("First newNode = %ld\n",(size_t)newNode);
    }else{

        fiberCnt++;

        cur->prev->next = newNode;
        newNode->next = cur;
        newNode->prev = cur->prev;
        cur->prev = newNode;

        printf("newNode = %ld\n",(size_t)newNode);
    }

    newNode->stack = malloc(4096);
    newNode->rsp = (size_t)((size_t*)newNode->stack+4088); //?????????

    //printf("made rsp = %ld\n",newNode->rsp);

    asm volatile(
        "push %%rax\n\t"
        "mov %%rsp, %%rax\n\t"
        "mov 0x0(%0), %%rsp\n\t"
        //"push %3\n\t"
        "push %2\n\t"
        "push %1\n\t"
        "mov %%rsp, 0x0(%0)\n\t"
        "mov %%rax, %%rsp\n\t"
        "pop %%rax\n\t"
        : "+r"(newNode)
        : "r"(data), "r"(func)//, "r"(&Handler)
        : "memory", "%rax"
    );
    //printf("made rsp after push = %ld\n",newNode->rsp);
    // *(size_t*)newNode->rsp = &Handler;
    // newNode->rsp-=8;
    // *(size_t*)newNode->rsp = func;
    // newNode->rsp-=8;
    // *(size_t*)newNode->rsp = data;
    // newNode->rsp-=8;
}

void Swap(struct Ctx* old, struct Ctx* new){
    //printf("new using1 =  %ld\n",(size_t)new);
    //printf("old using1 =  %ld\n",(size_t)old);
    printf("swapping from %d to %d\n",old->num, new->num);
    if (old->deleted==1){
        printf("=========deleting==========\n");
        Free(old);
        fiberCnt--;
    }else{
        asm volatile(
            
            "push %%rax\n\t"

            //"mov 0x8(%%rbp), %%rax\n\t" //rip
            //"mov %%rax, (%0)\n\t"

            // "mov (%%rbp), %%rax\n\t" //rbp
            // "mov (%%rax), %%rax\n\t"
            // "mov %%rax, 0x0(%0)\n\t"

            //"mov 0x16(%%rbp), %%rax\n\t" //rsp   или просто rsp положить?
            //"mov %%rax, 0x0(%0)\n\t"

            "mov %%rsp, 0x0(%0)\n\t" //rsp??

            "mov %%rbx, 0x8(%0)\n\t" //rbx

            "mov %%r12, 0x10(%0)\n\t" //r12

            "mov %%r13, 0x16(%0)\n\t" //r13

            "mov %%r14, 0x20(%0)\n\t" //r14

            "mov %%r15, 0x26(%0)\n\t" //r15

            "mov %%rbp, 0x30(%0)\n\t" //rbp

            "pop %%rax\n\t"

            :
            : "r"(old)
            : "%rax", "memory"
        );
    }
    //printf("new using2 =  %ld\n",(size_t)new);
    //printf("old using2 =  %ld\n",(size_t)old);
    if (new->started==0){
        new->started = 1;
        //printf("next rsp using =  %ld\n",new->rsp);
        asm volatile(
            "mov 0x0(%0), %%rsp\n\t"
            "pop %%rdi\n\t"
            "pop %%rsi\n\t"
            "call Handler\n\t"
            :
            : "r"(new)
            : "%rdi", "%rsi","memory"
        );
    }else{
        asm volatile(
            "mov 0x0(%0), %%rsp\n\t" //rsp

            "mov 0x8(%0), %%rbx\n\t" //rbx

            "mov 0x10(%0), %%r12\n\t" //r12

            "mov 0x16(%0), %%r13\n\t" //r13

            "mov 0x20(%0), %%r14\n\t" //r14

            "mov 0x26(%0), %%r15\n\t" //r15

            "mov 0x30(%0), %%rbp\n\t" //rbp
            :
            : "r"(new)
            : "%rax", "memory"
        );
    }
}
void FiberYield(){
    struct Ctx* old = cur;
    cur = cur->next;
    Swap(old,cur);
}

int FiberTryJoin(){
    if (fiberCnt==1){
        free(cur);
    }
    return (int)(fiberCnt==1);
}