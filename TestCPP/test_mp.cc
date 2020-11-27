#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/mman.h>
#include <execinfo.h>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <iostream>

static void sigsegv_handler(int signum, siginfo_t *info, void *data) {
    std::cout<<" Segment Fault"<<std::endl;
    void *addr = info->si_addr;
    char buff[256];
    int fd = open("./sigsegv.bt",O_CREAT|O_RDWR|O_APPEND);
    int len = snprintf(buff,256,"Addr: %p\n",addr);
    write(fd,buff,len);
    void* array[30];
    size_t size = backtrace(array,30);
    backtrace_symbols_fd(array,size,fd);
    close(fd);
}

void register_signal(int signum){
    struct sigaction action;
    bzero(&action, sizeof(action));
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO | SA_RESETHAND;
    action.sa_sigaction = &sigsegv_handler;
    sigaction(signum, &action, NULL);
}

void* core_func(long stack_size){
    void *stack = valloc(stack_size);
    mprotect(stack, getpagesize(), PROT_NONE);
    return stack;
}

void destroy(void* stack){
    mprotect(stack, getpagesize(), PROT_READ|PROT_WRITE);
    free(stack);
}


int main(int argc,char* argv[]){
    register_signal(SIGSEGV);
    void* stack = core_func(1024);
    std::cout<<"Write"<<std::endl;
    *(char *)(stack+10) = 'a';
    std::cout<<"Write Done"<<std::endl;
    destroy(stack);
}

