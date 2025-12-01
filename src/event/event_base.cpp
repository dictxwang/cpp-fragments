#include <iostream>
#include <event.h>
#include <signal.h>
#include <thread>
#include <chrono>
#include <queue>
#include <mutex>
#include <shared_mutex>

// Signal handler callback
void signal_cb(evutil_socket_t sig, short events, void *user_data) {
    struct event_base *base = (struct event_base *)user_data;
    std::cout << "Caught signal " << sig << " exiting event loop.\n";

    // Break the event loop
    event_base_loopbreak(base);
}

void signal_event_base() {

    // Initialize the event base
    struct event_base *base = event_base_new();
    if (base == nullptr) {
        std::cout << "Could not initialize libevent.\n";
        return;
    }
    
    // Create a signal event for SIGINT
    struct event *sigint_event = evsignal_new(base, SIGINT, signal_cb, base);
    if (sigint_event == nullptr || event_add(sigint_event, nullptr) < 0) {
        std::cout << "Could not create/add SIGINT event.\n";
        event_free(sigint_event);
        event_base_free(base);
        return;
    }
    
    std::cout << "Event loop running. Press Ctrl+C to exit.\n";

    // Run the event loop
    int ret = event_base_dispatch(base);
    if (ret == 0) {
        std::cout << "Event loop exited cleanly.\n";
    } else if (ret == -1) {
        std::cout << "Event loop exited with an error.\n";
    } else if (ret == 1) {
        std::cout << "Event loop exited prematurely.\n";
    }

    // After event loop. Cleanup
    event_free(sigint_event);
    event_base_free(base);
}

// Timer handler callback
void timer_cb(evutil_socket_t fd, short events, void *user_data) {
    int *times = (int*)user_data;
    std::this_thread::sleep_for(std::chrono::seconds(*times));
    std::cout << "Timer event triggered for " << *times << "\n";
}

void timer_event_base() {

    // Initialize the event base
    struct event_base *base = event_base_new();
    if (base == nullptr) {
        std::cout << "Could not initialize libevent.\n";
        return;
    }
    
    // Create a timer event
    struct timeval five_second = {5, 0}; // 5 second
    int times = 5;
    // struct event *timer_event = event_new(base, -1, EV_PERSIST, timer_cb, nullptr); // EV_PERSIST makes it repeat
    struct event *timer_event = evtimer_new(base, timer_cb, &times); // w rapper about event_new(), will not repeat
    event_add(timer_event, &five_second);

    // Start ebent loop
    std::cout << "Event loop running. Timer will trigger in 5 second.\n";
    event_base_dispatch(base);

    //After event loop. Cleanup
    event_free(timer_event);
    event_base_free(base);
}

void multi_event_base() {

    // Initialize the event base
    struct event_base *base = event_base_new();
    if (base == nullptr) {
        std::cout << "Could not initialize libevent.\n";
        return;
    }

    // Create a signal event for SIGINT
    struct event *sigint_event = evsignal_new(base, SIGINT, signal_cb, base);
    if (sigint_event == nullptr || event_add(sigint_event, nullptr) < 0) {
        std::cout << "Could not create/add SIGINT event.\n";
        event_free(sigint_event);
        return;
    }
    event_add(sigint_event, nullptr);

    // Create a timer event
    struct timeval three_second = {3, 0}; // 3 second
    int times_three = 3;
    struct event *timer_event = event_new(base, -1, EV_PERSIST, timer_cb, &times_three);
    event_add(timer_event, &three_second);

    struct timeval two_second = {2, 0}; // 2 second
    int times_two = 2;
    struct event *timer_event_another = event_new(base, -1, EV_PERSIST, timer_cb, &times_two);
    event_add(timer_event_another, &two_second);

    std::cout << "Event loop running. Press Ctrl+C to exit.\n";

    // Start event loop
    int ret = event_base_dispatch(base);
    if (ret == 0) {
        std::cout << "Event loop exited cleanly.\n";
    } else if (ret == -1) {
        std::cout << "Event loop exited with an error.\n";
    } else if (ret == 1) {
        std::cout << "Event loop exited prematurely.\n";
    }
}

void user_callback(evutil_socket_t fd, short what, void* arg) {
    std::cout << "User event triggered!\n";
    // what will be EV_READ when triggered
}

void user_event_base() {
    
    event_base* base = event_base_new();

    // Create a manual event (no fd, no timeout)
    event* user_event = event_new(base, -1, EV_PERSIST, user_callback, nullptr);
    event_add(user_event, nullptr);

    // Trigger from another thread
    std::thread t([base, user_event]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Thread-safe way to trigger event
        event_active(user_event, EV_READ, 0);

        // Or use event_base_once for thread safety
        struct timeval tv = {0, 0};
        event_base_once(base, -1, EV_TIMEOUT,
                        [](evutil_socket_t, short, void*) {
                            std::cout << "Triggered via event_base_once\n";
                        }, nullptr, &tv);
    });

    // Run for 2 seconds
    struct timeval tv = {2, 0};
    event_base_loopexit(base, &tv);
    event_base_dispatch(base);

    t.join();
    // std::this_thread::sleep_for(std::chrono::seconds(5));
    event_free(user_event);
    event_base_free(base);
}

struct EvWorkItem {
    std::function<void()> task;
};

class EventLoopWorker {
    event_base* base;
    event* work_event;
    std::queue<EvWorkItem> work_queue;
    std::mutex queue_mutex;

    static void work_callback(evutil_socket_t fd, short what, void *arg) {
        std::cout << "004" << std::endl;
        auto* worker = static_cast<EventLoopWorker*>(arg);
        std::cout << "005" << std::endl;
        worker->processWork();
    }
    public:
      EventLoopWorker() {
          base = event_base_new();
          work_event = evuser_new(base, work_callback, this);
            //   event_add(work_event, nullptr);
        if (work_event == nullptr || event_add(work_event, nullptr) < 0) {
            std::cout << "Could not create/add SIGINT event.\n";
            return;
        }
        event_add(work_event, nullptr);
      }
      void submitWork(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            work_queue.push({task});
        }
        if (work_event == nullptr) {
            std::cout << "Could not create/add SIGINT event.\n";
            return;
        }
        evuser_trigger(work_event);
      }
      void processWork() {
          std::vector<EvWorkItem> items;
          {
              std::lock_guard<std::mutex> lock(queue_mutex);
              while (!work_queue.empty()) {
                  items.push_back(work_queue.front());
                  work_queue.pop();
              }
          }
          std::cout << "005" << std::endl;

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

    // signal_event_base();
    // timer_event_base();
    // multi_event_base();
    // user_event_base();
    
    EventLoopWorker worker;
    std::thread worker_thread([&worker]() { worker.run(); });
    std::this_thread::sleep_for(std::chrono::seconds(2));

    worker.submitWork([]() { std::cout << "Task 1\n"; });
    worker.submitWork([]() { std::cout << "Task 2\n"; });
    return 0;
}