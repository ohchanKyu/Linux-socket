#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>

int main() {
    // initialize random number generator
    srand(time(NULL));

    // create a ZeroMQ context
    void *context = zmq_ctx_new();

    // create a PUB (publisher) socket and bind it to a port
    void *socket = zmq_socket(context, ZMQ_PUB);
    zmq_bind(socket, "tcp://*:5000");

    printf("Publisher started, broadcasting on port 5000...\n");

    while (1) {
        // generate a random topic
        const char *topics[] = {"sports", "news", "weather"};
        const char *topic = topics[rand() % 3];

        // create the message
        char message[256];
        time_t now = time(NULL);
        snprintf(message, sizeof(message), "%s - Message at %s", topic, ctime(&now));

        // remove newline character from ctime output
        message[strcspn(message, "\n")] = '\0';

        // publish the message
        printf("Publishing: %s\n", message);
        zmq_send(socket, message, strlen(message), 0);

        // wait for a second before sending the next message
        sleep(1);
    }

    // close the socket and terminate the context
    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}
