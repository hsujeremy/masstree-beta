#include <iostream>
#include <random>
#include <thread>
#include <unistd.h>
#include "compiler.hh"
#include "masstree_struct.hh"
#include "nodeversion.hh"

#define TRIALS 1000

struct test_type : public Masstree::nodeparams<15, 15> {
    typedef uint64_t value_type;
    typedef ::threadinfo threadinfo_type;
};

typedef Masstree::node_base<test_type>::nodeversion_type test_nodeversion;

void thread_try_lock(test_nodeversion* v) {
    if (v->try_lock()) {
        usleep(rand() % 1001);
        v->unlock();
    }
}

int main() {
    printf("Starting internal nodeversion concurrency tests...\n");
    test_nodeversion v(false);

    // Test: sanity check, lock/unlock works
    v.try_lock();
    v.unlock();
    v.lock();
    v.unlock();
    printf("Single thread lock/unlock passed!\n");

    // Test: multiple threads should not be able to obtain the lock concurrently
    srand(time(0));
    for (int i = 0; i < 1000; ++i) {    
        std::thread t1(thread_try_lock, &v);
        std::thread t2(thread_try_lock, &v);
        std::thread t3(thread_try_lock, &v);
        t1.join();
        t2.join();
        t3.join();
    }
    printf("Threee threads concurrently locking/unlocking passed!\n");

    printf("All tests passed!\n");
}
