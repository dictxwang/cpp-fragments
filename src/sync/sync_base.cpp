#include <barrier>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;



int main(int argc, char const *argv[])
{
    const int workder_number = 4;
    std::barrier sync_point(workder_number);

    auto worker = [&](int id, int duration) {
        std::cout << "worker-" << id << " start." << std::endl;
        this_thread::sleep_for(chrono::seconds(duration));
        std::cout << "worker-" << id << " finish." << std::endl;
        sync_point.arrive_and_wait();
    };

    for (int i = 0; i < workder_number; ++i) {
        thread t(worker, i, 5);
    }

    std::cout << "this is main thread" << std::endl;

    /* code */
    return 0;
}
