#ifndef _CONCURRENT_LOCK_H_
#define _CONCURRENT_LOCK_H_
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

using namespace std;

mutex mtx;
condition_variable cv;
queue<int> buffer;

void producer(int id) {
    for (int i = 0; i < 5; ++i) {
        unique_lock<mutex> lock(mtx);
        buffer.push(i);
        cout << ">>> Producer " << id << " produced: " << i << endl;
        cv.notify_one();
        lock.unlock();
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}
void consumer(int id) {
    while(true) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [] { return !buffer.empty(); });
        if (!buffer.empty()) {
            int item = buffer.front();
            buffer.pop();
            cout << ">>> Consumer " << id << " consumed: " << item << endl;
        }
        lock.unlock();
    }
}
#endif