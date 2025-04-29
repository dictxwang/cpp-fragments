#include <iostream>
#include <event.h>

// Signal handler callback
void signal_cb(evutil_socket_t sig, short events, void *user_data) {
    struct event_base *base = (struct event_base *)user_data;
    std::cout << "Caught signal " << sig << "exiting event loop.\n";

    // Break the event loop
    event_base_loopbreak(base);
}

int main() {
    // Initialize an event base
    struct event_base *base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent.\n");
        return 1;
    }

    // Create a signal event for SIGINT
    struct event *sigint_event = evsignal_new(base, SIGINT, signal_cb, base);
    if (!sigint_event || event_add(sigint_event, NULL) < 0) {
        fprintf(stderr, "Could not create/add SIGINT event.\n");
        event_base_free(base);
        return 1;
    }

    printf("Event loop running. Press Ctrl+C to exit.\n");

    // Run the event loop
    int ret = event_base_dispatch(base);
    if (ret == 0) {
        printf("Event loop exited cleanly.\n");
    } else if (ret == -1) {
        printf("Event loop exited with an error.\n");
    } else if (ret == 1) {
        printf("Event loop exited prematurely.\n");
    }

    // Cleanup
    event_free(sigint_event);
    event_base_free(base);

    return 0;
}