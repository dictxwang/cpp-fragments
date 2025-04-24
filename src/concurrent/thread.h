#ifndef _CONCURRENT_THREAD_H_
#define _CONCURRENT_THREAD_H_

#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

void print_delay_message(const string& message, int delay) {
    this_thread::sleep_for(chrono::seconds(delay));
    cout << message << endl;
}

#endif