#ifndef TEST_THREAD_H
#define TEST_THREAD_H

#include<pthread.h>
#include<sys/types.h>

class Thread{
    private:
    pthread_t thread_id;
    pid_t pid;
    const char*  thread_name;
    void* entry_wrapper();

    public:
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread();
    virtual ~Thread();

    protected:
    virtual void * entry()=0;
    private:
    static void * _entry_func(void* arg);

    public:
    const pthread_t& get_thread_id() const;
    bool is_start()const;
    int kill(int signal);
    void create(const char* name,size_t stacksize = 0);
    int try_create(size_t stacksize);
    int join(void **prval = 0);
    int detach();

};

#endif