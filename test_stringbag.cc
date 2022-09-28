#include <assert.h>
#include <iostream>
#include <thread>
#include "stringbag.hh"

#define WIDTH 15

static int count1 = 0;
static int count2 = 0;

void fill_bag1(stringbag<uint16_t>* sbptr) {
    printf("Thread 1 filling stringbag...\n");
    for (int i = 1; i < WIDTH; i += 2) {
        if (!sbptr->filled(i)) {
            if (!sbptr->assign(i, lcdf::Str("aaa", 3))) {
                break;
            }
            ++count1;
        }
    }
}

void fill_bag2(stringbag<uint16_t>* sbptr) {
    printf("Thread 2 filling stringbag...\n");
    for (int i = 0; i < WIDTH; i += 2) {
        if (!sbptr->filled(i)) {
            if (!sbptr->assign(i, lcdf::Str("bbb"))) {
                break;
            }
            ++count2;
        }
    }
}

void check_results(stringbag<uint16_t>* sbptr) {
    printf("Checking results\n");
    assert(count1 + count2 == WIDTH);
    int local_count1 = 0;
    int local_count2 = 0;
    lcdf::Str str = (*sbptr)[0];
    for (int i = 0; i < WIDTH; ++i) {
        local_count1 += !strncmp("aaa", (*sbptr)[i].s, 3);
        local_count2 += !strncmp("bbb", (*sbptr)[i].s, 3);
    }
    assert(local_count1 + local_count2 == WIDTH);
    assert(local_count1 == count1 && local_count2 == count2);
}

int main() {
    // Construct a stringbag object with space for `WIDTH` characters
    size_t safe_sz = stringbag<uint16_t>::safe_size(WIDTH, 64);
    printf("safe size: %zu, max size: %u\n", safe_sz,
           stringbag<uint16_t>::max_size());
    stringbag<uint16_t>* sbptr = (stringbag<uint16_t>*) malloc(safe_sz);
    new((void*) sbptr) stringbag<uint16_t>(WIDTH, safe_sz);

    // Spin off two threads to fill stringbag
    printf("Creating two threads to fill stringbag...\n");
    std::thread thread1(fill_bag1, std::ref(sbptr));
    std::thread thread2(fill_bag2, std::ref(sbptr));

    // Join once stringbag is full
    thread1.join();
    thread2.join();

    printf("count1: %d, count2: %d\n", count1, count2);
    sbptr->print(WIDTH, stdout, "", 0);

    // Spin off third thread to check stringbag contents
    std::thread thread3(check_results, std::ref(sbptr));
    thread3.join();
    printf("All checks passed!\n");
}
