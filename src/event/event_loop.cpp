#include <iostream>
#include <event2/event.h>
#include <signal.h>
#include <thread>
#include <chrono>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <vector>
#include <unistd.h>

struct WorkItem {
    std::function<void()> task;
};

class EventLoopWorker {
    event_base* base;
    event* work_event;
    std::queue<WorkItem> work_queue;
    std::mutex queue_mutex;

    static void work_callback(evutil_socket_t fd, short what, void* arg) {
        auto* worker = static_cast<EventLoopWorker*>(arg);
        worker->processWork();
    }

public:
    EventLoopWorker() {
        base = event_base_new();
        work_event = evuser_new(base, work_callback, this);
        event_add(work_event, nullptr);
    }
    ~EventLoopWorker() {
          if (base) {
              event_base_free(base);
          }
      }

    void submitWork(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            work_queue.push({task});
        }
        evuser_trigger(work_event);
    }

    void processWork() {
        std::vector<WorkItem> items;
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            while (!work_queue.empty()) {
                items.push_back(work_queue.front());
                work_queue.pop();
            }
        }

        for (auto& item : items) {
            item.task();
        }
    }

    void run() {
        event_base_dispatch(base);
    }

    void stop() {
        event_base_loopbreak(base);
    }
};

int main() {
    // Usage
    EventLoopWorker worker;
    std::thread worker_thread([&worker]() { worker.run(); });

    worker.submitWork([]() { std::cout << "Task 1\n"; });
    worker.submitWork([]() { std::cout << "Task 2\n"; });

    std::this_thread::sleep_for(std::chrono::seconds(200));

    worker.stop();
    worker_thread.join();

    return 0;
}