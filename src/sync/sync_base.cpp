#include <barrier>
#include <thread>
#include <chrono>
#include <vector>
#include <iostream>
#include <latch>

using namespace std;

void print_worker() {
    while (true) {
        this_thread::sleep_for(chrono::seconds(1));
        std::cout << "this is print worker" << std::endl;
    }
}

void use_barrier() {
    const int workder_number = 4;
    std::barrier sync_point(workder_number);

    auto worker = [&](int id, int duration) {
        std::cout << "barrier worker-" << id << " start." << std::endl;
        this_thread::sleep_for(chrono::seconds(duration));
        std::cout << "barrier worker-" << id << " finish." << std::endl;
        sync_point.arrive_and_wait();
    };

    vector<thread> threads;

    for (int i = 0; i < workder_number; ++i) {
        threads.emplace_back(worker, i, (i+1) * 3);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "barrier all workers done" << std::endl;
}

void use_latch() {

    // std::latch start_signal(1);
    std::latch done_signal(3);

    auto worker_latch = [&](int id) {
        // start_signal.wait();  // Wait for start
        std::cout << "latch worker-" << id << " starts" << std::endl;
        this_thread::sleep_for(chrono::seconds(3*id));
        std::cout << "latch worker-" << id << " finished" << std::endl;
        done_signal.count_down();  // Signal done
    };

    vector<thread> threads;

    for (int i = 1; i <= 3; i++) {
        threads.emplace_back(worker_latch, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "latch starting all workers..." << std::endl;
    // start_signal.count_down();  // Release all workers

    done_signal.wait();  // Wait for all to finish
    std::cout << "latch all workers done" << std::endl;
}


int main(int argc, char const *argv[])
{
    thread latch_thread(use_latch);
    latch_thread.detach();

    thread barrier_thread(use_barrier);
    barrier_thread.detach();

    thread print_thread(print_worker);
    print_thread.detach();

    std::cout << "this is main thread" << std::endl;

    while(true) {
        cout << "Keep Running..." << endl;
        std::this_thread::sleep_for(chrono::seconds(3));
    }

    /* code */
    return 0;
}
