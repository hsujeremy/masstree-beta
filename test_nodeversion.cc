#include <iostream>
#include <thread>
#include "compiler.hh"
#include "masstree_struct.hh"
#include "nodeversion.hh"

struct test_type : public Masstree::nodeparams<15, 15> {
    typedef uint64_t value_type;
    typedef ::threadinfo threadinfo_type;
};

void thread_try_lock(Masstree::node_base<test_type>::nodeversion_type *v) {
    printf("Addr of nodeversion within thread %p\n", v);
    if (v->try_lock()) (*v).unlock();
}

int main() {
    printf("Starting nodeversion concurrency tests\n");

    // TODO: assertion error hits with this line
    Masstree::node_base<test_type>::nodeversion_type v;

    printf("Test: single-threaded lock/unlock works... ");
    v.try_lock();
    v.unlock();
    printf("Passed\n");

    // Make sure two threads cannot acquire the lock at the same time
    printf("try lock with two threads\n");
    std::thread t1(thread_try_lock, &v);
    std::thread t2(thread_try_lock, &v);
    t1.join();
    t2.join();
}
