#include"Thread.h"
//#ifdef HAVE_SCHED
//#include<sched.h>
//#endif
#include <sys/syscall.h>

Thread::Thread()
    : thread_id(0),
    pid(0),
    thread_name(NULL)
    {

    }
Thread::~Thread(){

}

void* Thread::_entry_func(void *arg){
    void *r = ((Thread*)arg)->entry_wrapper;
    return r;
}

void *Thread::entry_wrapper(){
    int p = syscall(SYS_gettid);
    pthread_setname_np(thread_name);
    return entry();
}

