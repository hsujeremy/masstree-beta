#include <algorithm>
#include <assert.h>
#include <deque>
#include <iostream>
#include <queue>
#include <random>
#include <stdlib.h>
#include <time.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include "stringbag.hh"

#define WIDTH 15
#define TRIALS 1000
#define VERBOSE 0

std::atomic<bool> finished_writing = {false}; // better than fill?

static lcdf::Str strings[10] = {
    lcdf::Str("aaa", 3), lcdf::Str("bbb", 3), lcdf::Str("ccc", 3),
    lcdf::Str("adabc", 5), lcdf::Str("make", 4), lcdf::Str("items", 5),
    lcdf::Str("tree", 4), lcdf::Str("cs161", 5), lcdf::Str("cs165", 5),
    lcdf::Str("lgtm", 4)
};

// -----------------------------------------------------------------------------
// New tests for MRSW concurrency

void seqwriter(stringbag<uint16_t>* sbptr) {
    for (int i = 0; i < WIDTH; ++i) {
        usleep(rand() % 1001);
        int si = rand() % 10;
        lcdf::Str s = strings[si];
        if (!sbptr->assign(i, s)) {
            return;
        }
    }
    finished_writing.store(true, std::memory_order_seq_cst);
}

void randwriter(stringbag<uint16_t>* sbptr, std::default_random_engine* re) {
    std::deque<int> ordered_slots;
    for (int i = 0; i < WIDTH; ++i) {
        ordered_slots.push_back(i);
    }
    std::shuffle(ordered_slots.begin(), ordered_slots.end(), *re);
    std::queue<int> open_slots(ordered_slots);
    while (!open_slots.empty()) {
        // usleep(rand() % 1001);
        int p = open_slots.front();
        int si = rand() % 10;
        lcdf::Str s = strings[si];
        if (VERBOSE) {
            printf("inserting %d %.*s\n", p, s.len, s.s);
        }
        if (!sbptr->assign(p, s)) {
            break;
        }
        open_slots.pop();
    }
    finished_writing.store(true, std::memory_order_seq_cst);
}

void reader(stringbag<uint16_t>* sbptr) {
    std::unordered_map<int, lcdf::Str> seen;
    size_t iter = 0;
    while (!finished_writing.load(std::memory_order_seq_cst)) {
        ++iter;
        for (int i = 0; i < WIDTH; ++i) {
            bool r = sbptr->filled(i);
            if (r) {
                lcdf::Str item = (*sbptr)[i];
                if (seen.find(i) == seen.end()) {
                    if (VERBOSE) {
                        printf("read inserting %.*s\n", item.len, item.s);
                    }
                    seen.insert({i, item});
                } else {
                    lcdf::Str expected = seen[i];
                    if (VERBOSE && strncmp(item.s, expected.s, item.len)) {
                        printf("Got %.*s but expected %s\n", item.len, item.s,
                               expected.s);
                        assert(false);
                    }
                    assert(!strncmp(item.s, expected.s, item.len));
                }
            }
        }
    }
    if (VERBOSE) {
        printf("looped %zu times before filled\n", iter);
    }
}

int main() {
    printf("Starting stringbag concurrency tests...\n");
    for (int i = 0; i < TRIALS; ++i) {
        size_t safe_sz = stringbag<uint16_t>::safe_size(WIDTH, 64);
        stringbag<uint16_t>* sbptr = (stringbag<uint16_t>*) malloc(safe_sz);
        new((void*) sbptr) stringbag<uint16_t>(WIDTH, safe_sz);

        srand(time(0));
        finished_writing.store(false, std::memory_order_seq_cst);
        std::thread t1(seqwriter, std::ref(sbptr));
        std::thread t2(reader, std::ref(sbptr));
        std::thread t3(reader, std::ref(sbptr));
        t1.join();
        t2.join();
        t3.join();

        free(sbptr);
    }
    printf("MRSW with sequential position writes passed!\n");

    auto re = std::default_random_engine(time(0));
    for (int i = 0; i < TRIALS; ++i) {
        size_t safe_sz = stringbag<uint16_t>::safe_size(WIDTH, 64);
        stringbag<uint16_t>* sbptr = (stringbag<uint16_t>*) malloc(safe_sz);
        new((void*) sbptr) stringbag<uint16_t>(WIDTH, safe_sz);

        srand(time(0));
        finished_writing.store(false, std::memory_order_seq_cst);
        std::thread t1(randwriter, std::ref(sbptr), &re);
        std::thread t2(reader, std::ref(sbptr));
        std::thread t3(reader, std::ref(sbptr));
        t1.join();
        t2.join();
        t3.join();

        free(sbptr);
    }
    printf("MRSW with random position writes passed!\n");
    printf("All checks passed!\n");
    return 0;
}
