#include <iostream>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <condition_variable>

class thr{
private:
    std::mutex mtx;
    std::condition_variable cond;

    std::thread m_handle;

    bool stopped=false;

public: 
    virtual void process()=0;

public:
    virtual void run(thr* next_ptr){
        std::unique_lock<std::mutex> lk{mtx};
        while (!isstop()){
            cond.wait(lk);
            if (stopped){
                return;
            }
            process();
            next_ptr->notify();
        }
    }

public:
    void create(thr* next_ptr){
        m_handle = std::thread(&thr::run,this,next_ptr);
    }

    void join(){
        if (m_handle.joinable())
            m_handle.join();
    }
    void notify(){
        cond.notify_all();
    }

    void stop(){
        std::lock_guard<std::mutex> lock{mtx};
        stopped = true;
        cond.notify_all();
    }

    bool isstop(){
        return stopped == true;
    }
};

class A:public thr{

public:
    void process() override{
        printf("a");
    }

};

class B:public thr{

public:
    void process() override{
        printf("b");
    }
};


class C:public thr{

public:
    void process() override{
        printf("c");
        printf("\n=========\n");
    }

};


int main(int argc, char* argv[]){
    A* ptr_a = new A();
    B* ptr_b = new B();
    C* ptr_c = new C();
    printf("Start: \n");
    ptr_a->create(ptr_b);
    ptr_b->create(ptr_c);
    ptr_c->create(ptr_a);
    ptr_a->notify();
    sleep(3);
    ptr_a->stop();
    ptr_b->stop();
    ptr_c->stop();
    ptr_a->join();
    ptr_b->join();
    ptr_c->join();
    delete ptr_a;
    delete ptr_b;
    delete ptr_c;
}