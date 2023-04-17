#include <assert.h>
#include <iostream>
#include <random>
#include <thread>
#include <unistd.h>
#include "compiler.hh"
#include "masstree_struct.hh"
#include "nodeversion.hh"

#define TRIALS 1000
#define THREADS 5

// TODO: root, deleted, has_changed tests
// TODO: extend to test 32-bit version

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

void thread_mark_generic(test_nodeversion* v, void (*setfn)(test_nodeversion*), 
                         void (*checkfn)(test_nodeversion*, bool)) {
    if (!v->try_lock()) {
        return;
    }
    checkfn(v, false);
    test_nodeversion initv = *v;
    setfn(v);
    checkfn(v, true);
    v->assign_version(initv);
    checkfn(v, false);
    v->unlock();
}

void thread_mark_insert(test_nodeversion* v) {
    auto setfn = [](test_nodeversion* v) { 
        v->mark_insert(); 
    };
    auto checkfn = [](test_nodeversion* v, bool expect_set) {
        assert(v->inserting() == expect_set);
    };
    thread_mark_generic(v, setfn, checkfn);
}

void thread_mark_split(test_nodeversion* v) {
    auto setfn = [](test_nodeversion* v) { 
        v->mark_split(); 
    };
    auto checkfn = [](test_nodeversion* v, bool expect_set) {
        assert(v->splitting() == expect_set);
    };
    thread_mark_generic(v, setfn, checkfn);
}

void thread_mark_root_nonroot(test_nodeversion* v) {
    if (!v->try_lock()) {
        return;
    }
    assert(!v->is_root());
    v->mark_root();
    assert(v->is_root());
    v->mark_nonroot();
    assert(!v->is_root());
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

    std::thread threads[THREADS];

    // Test: multiple threads should not be able to obtain the lock concurrently
    srand(time(0));
    for (int i = 0; i < TRIALS; ++i) {   
        for (int j = 0; j < THREADS; ++j) {
            threads[j] = std::thread(thread_try_lock, &v);
        }
        for (int j = 0; j < THREADS; ++j) {
            threads[j].join();
        }
    }
    printf("Concurrent locking/unlocking test passed!\n");

    // Test: multiple threads shouldn't be able to set/clear inserting bit
    // concurrently
    for (int i = 0; i < TRIALS; ++i) {
        for (int j = 0; j < THREADS; ++j) {
            threads[j] = std::thread(thread_mark_insert, &v);
        }
        for (int j = 0; j < THREADS; ++j) {
            threads[j].join();
        }
    }
    printf("Concurrent mark/clear insert bit test passed!\n");

    // Test: multiple threads shouldn't be able to set/clear splitting bit
    // concurrently
    for (int i = 0; i < TRIALS; ++i) {
        for (int j = 0; j < THREADS; ++j) {
            threads[j] = std::thread(thread_mark_split, &v);
        }
        for (int j = 0; j < THREADS; ++j) {
            threads[j].join();
        }
    }
    printf("Concurrent mark/clear split bit test passed!\n");

    // Test: multiple threads shouldn't be able to mark the node as root or
    // nonroot concurrently
    for (int i = 0; i < TRIALS; ++i) {
        for (int j = 0; j < THREADS; ++j) {
            threads[j] = std::thread(thread_mark_root_nonroot, &v);
        }
        for (int j = 0; j < THREADS; ++j) {
            threads[j].join();
        }
    }
    printf("Concurrent mark root/nonroot test passed!\n");

    printf("All tests passed!\n");
}
