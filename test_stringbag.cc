#include <assert.h>
#include <iostream>
#include <thread>

// Create an empty stringbag structure
// Have two threads concurrently write to stringbag until it's full
// Once it's full, spin up a third thread and check results

void f1() {
    printf("Calling from thread 1\n");
}

void f2() {
    printf("Calling from thread 2\n");
}

void f3() {
    printf("Calling from thread 3\n");
}

int main() {
    // Spin off two threads to fill stringbag
    std::thread thread1(f1);
    std::thread thread2(f2);

    // Join once stringbag is full
    thread1.join();
    thread2.join();

    // Spin off third thread to check stringbag contents
    std::thread thread3(f3);
    thread3.join();
    printf("Finished running threads\n");
}
