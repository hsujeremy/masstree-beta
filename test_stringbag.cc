#include <assert.h>
#include <iostream>
#include <thread>
#include "stringbag.hh"

// Create an empty stringbag structure
// Have two threads concurrently write to stringbag until it's full
// Once it's full, spin up a third thread and check results

#define WIDTH 15

static int count1 = 0;
static int count2 = 0;

void fill_bag1(stringbag<uint16_t>* sb) {
    printf("Calling from thread 1 %p\n", &sb);
    for (int i = 1; i < WIDTH; i += 2) {
        if (!sb->filled(i)) {
            if (!sb->assign(i, lcdf::Str("aaa"))) {
                break;
            }
            ++count1;
        }
    }
    printf("count1: %d\n", count1);
}

void fill_bag2(stringbag<uint16_t>* sb) {
    printf("Calling from thread 2 %p\n", &sb);
    for (int i = 0; i < WIDTH; i += 2) {
        if (!sb->filled(i)) {
            if (!sb->assign(i, lcdf::Str("bbb"))) {
                break;
            }
            ++count2;
        }
    }
    printf("count2: %d\n", count2);
}

void f3() {
    printf("Calling from thread 3\n");
}

int main() {
    // Construct a stringbag object with space for `WIDTH` characters
    size_t safe_sz = stringbag<uint16_t>::safe_size(WIDTH, 64);
    printf("safe size: %zu, max size: %u\n", safe_sz,
           stringbag<uint16_t>::max_size());
    stringbag<uint16_t>* sbptr = (stringbag<uint16_t>*) malloc(safe_sz);
    new((void*) sbptr) stringbag<uint16_t>(WIDTH, safe_sz);

    // Spin off two threads to fill stringbag
    printf("Passing to thread: %p\n", sbptr);
    std::thread thread1(fill_bag1, std::ref(sbptr));
    std::thread thread2(fill_bag2, std::ref(sbptr));

    // Join once stringbag is full
    thread1.join();
    thread2.join();

    printf("count1: %d, count2: %d\n", count1, count2);
    sbptr->print(WIDTH, stdout, "", 0);

    // Spin off third thread to check stringbag contents
    std::thread thread3(f3);
    thread3.join();
    printf("Finished running threads\n");
}
