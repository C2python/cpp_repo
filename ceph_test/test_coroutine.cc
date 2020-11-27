#include <thread>
#include <chrono>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/yield.hpp>
#include <cstdio>

using namespace std::chrono_literals;
using coroutine = boost::asio::coroutine;


class task; // Forward declare both because they should know about each other
void foo(task &task, int &result);

// Common practice is to subclass coro
class task : coroutine {
    // All reused variables should not be local or they will be
    // re-initialized
    int result;

    void start() {
        // In order to actually begin, we need to "invoke ourselves"
        (*this)();
    }

    // Actual task implementation
    void operator()() {
        // Reenter actually manages the jumps defined by yield
        // If it's executed for the first time, it will just run from the start
        // If it reenters (aka, yield has caused it to stop and we re-execute)
        // it will jump to the right place for you
        reenter(this) {
            // Yield will store the current location, when reenter
            // is ran a second time, it will jump past yield for you
            yield foo(*this, result);
            std::printf("%d\n", result)
        }
    }
}

// Our longer task
void foo(task & t, int & result) {
    std::thread([&](){
        std::this_thread::sleep_for(1s);
        result = 3;
        // The result is done, reenter the task which will go to just after yield
        // Keep in mind this will now run on the current thread
        t();
    }).detach();
}

int main(int, const char**) {
    task t;

    // This will start the task
    t.start();

    std::thread([](){
        std::this_thread::sleep_for(2s);
    }).join();
    return 0;
}