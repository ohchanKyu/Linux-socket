#include <stdio.h>
#include <string.h>
#include <zmq.h>

int main(int argc, char *argv[]) {
    // check if a topic is provided; default to "sports" if not
    const char *topic = (argc > 1) ? argv[1] : "sports";

    // create a ZeroMQ context
    void *context = zmq_ctx_new();

    // create a SUB (subscriber) socket and connect to the publisher
    void *socket = zmq_socket(context, ZMQ_SUB);
    zmq_connect(socket, "tcp://localhost:5556");

    printf("Connected to publisher on port 5556, subscribing to '%s'\n", topic);

    // subscribe to the specified topic
    zmq_setsockopt(socket, ZMQ_SUBSCRIBE, topic, strlen(topic));

    while (1) {
        // receive messages that match the subscribed topic
        char message[256];
        zmq_recv(socket, message, sizeof(message) - 1, 0);
        message[255] = '\0'; // ensure null-termination
        printf("Received: %s\n", message);
    }

    // close the socket and terminate the context
    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}
