#include "backtrace.h"

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <string.h>

typedef struct{
    void* from;
    void* to;
    char* name;
}SymInfo;


void GetSynTable(SymInfo* info,size_t* sz){
    int fd = open("/proc/self/exe", O_RDONLY);
    off_t offset = 0;
    void* addr = NULL;
    off_t len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    addr = mmap(NULL,len,PROT_READ,MAP_PRIVATE,fd,offset);
    if (addr == MAP_FAILED){
        close(fd);
        perror("Elf mapping failed\n");
    }
    Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)addr;

    Elf64_Shdr* elfSectionHeader = (Elf64_Shdr*)(addr+elfHeader->e_shoff);
    Elf64_Sym* elfSymbolTable = NULL;
    char* elfStrTable = NULL;
    size_t symSize = 0;
    for (size_t i = 0; i < elfHeader->e_shnum; i++)
    {
        if (elfSectionHeader[i].sh_type==SHT_SYMTAB){
            elfSymbolTable = (Elf64_Sym*) (addr+elfSectionHeader[i].sh_offset);
            size_t ind = elfSectionHeader[i].sh_link;
            elfStrTable = (char*) (addr+elfSectionHeader[ind].sh_offset);
            symSize = elfSectionHeader[i].sh_size/sizeof(Elf64_Sym);
        
        }
    }
    int ind = 0;
    for (size_t i = 0; i < symSize; i++)
    {
        if (elfSymbolTable[i].st_value!=0){
            //info[ind].name = elfSymbolTable[i].st_name+elfStrTable;
            info[ind].name = strdup(elfSymbolTable[i].st_name+elfStrTable);
            info[ind].from = (void*)elfSymbolTable[i].st_value;
            info[ind].to = (void*)elfSymbolTable[i].st_value + elfSymbolTable[i].st_size;
            if (strcmp("Func1",info[ind].name)==0){
                //printf("%p",info[ind].from);
            }
            //printf("%ld",elfSymbolTable[i].st_size);
            ind++;
        }
    }
    *sz = ind;
    munmap(addr,len);
    close(fd);
}
void Free(SymInfo* info,size_t sz){
    for (size_t i = 0; i < sz; i++)
    {
        free(info[i].name);
    }
    
}
char ans[200];
char* AddrToName(void* addr) {
    SymInfo info[2000];
    size_t sz = 0;
    GetSynTable(info, &sz);
    for (size_t i = 0; i < sz; i++)
    {
        //printf("%p  ",info[i].from);
        //printf("%p\n",info[i].to);

        if (addr>=info[i].from && addr<info[i].to){
            //printf("%s\n",info[i].name);
            strcpy(ans,info[i].name);
            Free(info,sz);
            return ans;
        }
    }
    Free(info,sz);
    //printf("%p  \n",addr);
    return NULL;
}

int Backtrace(void* backtrace[], int limit)  {
    void* framePtr;
    asm volatile ("mov %%rbp, %0\n\t"
    : "=r" (framePtr)
    : 
    :
    );
    int ind = 0;
    while (framePtr != NULL) {
        void* return_address;
        asm volatile (
            "mov %1, %%rax\n\t"
            //"add $-8, %%rax\n\t"
            "mov 0x8(%%rax), %0\n\t"
            "mov 0x0(%%rax), %1\n\t"
            : "=r" (return_address), "+r" (framePtr)
            : 
            : "%rax"
            );

        char* name = AddrToName(return_address);
        backtrace[ind++] = return_address;
        if (strcmp(name,"main")==0){
            return ind;
        }
        //printf("%s\n",AddrToName(return_address));
    }

    return 0;
}

void PrintBt() {
    void* buff[256];
    int sz = Backtrace(buff, 256);
    for (size_t i = 0; i < sz; i++)
    {
        printf("%p %s\n",buff[i],AddrToName(buff[i]));
    }
    
}
