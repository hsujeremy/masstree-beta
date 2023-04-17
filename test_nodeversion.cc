#include <iostream>
#include <thread>
#include "compiler.hh"
#include "masstree_struct.hh"
#include "nodeversion.hh"

struct test_type : public Masstree::nodeparams<15, 15> {
    typedef uint64_t value_type;
    typedef ::threadinfo threadinfo_type;
};

typedef Masstree::node_base<test_type>::nodeversion_type test_nodeversion;

void thread_try_lock(test_nodeversion* v) {
    if (v->try_lock()) {
        v->unlock();
    }
}

int main() {
    printf("Starting nodeversion concurrency tests\n");

    test_nodeversion v(false);
    // printf("v_ is %u\n", v.version_value());

    printf("Test: single-threaded lock/unlock... ");
    v.try_lock();
    v.unlock();
    printf("Passed\n");

    // Make sure two threads cannot acquire the lock at the same time
    printf("Test: two threads, concurrent lock/unlock... ");
    std::thread t1(thread_try_lock, &v);
    std::thread t2(thread_try_lock, &v);
    t1.join();
    t2.join();
    printf("Passed\n");
}
