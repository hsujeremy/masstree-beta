#include <assert.h>
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

void thread_mark_insert(test_nodeversion* v) {
    if (!v->try_lock()) {
        return;
    }
    assert(!v->inserting());
    test_nodeversion initv = *v;
    v->mark_insert();
    assert(v->inserting());
    v->assign_version(initv);
    assert(!v->inserting());
    v->unlock();
}

int main() {
    printf("Starting internal nodeversion concurrency tests...\n");
    test_nodeversion v(false);

    // Test: sanity check, lock/unlock works
    v.try_lock();
    v.unlock();
    v.lock();
    v.unlock();
    printf("Single thread lock/unlock test passed!\n");

    // Test: multiple threads should not be able to obtain the lock concurrently
    srand(time(0));
    for (int i = 0; i < TRIALS; ++i) {    
        std::thread t1(thread_try_lock, &v);
        std::thread t2(thread_try_lock, &v);
        std::thread t3(thread_try_lock, &v);
        std::thread t4(thread_try_lock, &v);
        std::thread t5(thread_try_lock, &v);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
    }
    printf("Concurrent locking/unlocking test passed!\n");

    // Test: multiple threads shouldn't be able to set/clear inserting bit
    // concurrently
    for (int i = 0; i < TRIALS; ++i) {
        std::thread t1(thread_mark_insert, &v);  
        std::thread t2(thread_mark_insert, &v);
        std::thread t3(thread_mark_insert, &v);
        std::thread t4(thread_mark_insert, &v);
        std::thread t5(thread_mark_insert, &v);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
    }
    printf("Concurrent mark/clear insert bit test passed!\n");

    printf("All tests passed!\n");
}
