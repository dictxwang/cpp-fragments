#include <iostream>
#include <event.h>
#include <signal.h>

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
    std::cout << "Timer event triggered.\n";
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
    // struct event *timer_event = event_new(base, -1, EV_PERSIST, timer_cb, nullptr); // EV_PERSIST makes it repeat
    struct event *timer_event = evtimer_new(base, timer_cb, nullptr); // w rapper about event_new(), will not repeat
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
    struct timeval three_second = {3, 0}; // 5 second
    struct event *timer_event = event_new(base, -1, EV_PERSIST, timer_cb, nullptr);
    event_add(timer_event, &three_second);

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

int main() {

    // signal_event_base();
    // timer_event_base();

    multi_event_base();
    return 0;
}