#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

int main() {
    // create a ZeroMQ context
    void *context = zmq_ctx_new();

    // create a REP (reply) socket and bind it to a port
    void *socket = zmq_socket(context, ZMQ_REP);
    zmq_bind(socket, "tcp://*:5000");

    printf("Listening on port 5000...\n");

    while (1) {
        // wait for a request from the client
        char message[256];
        zmq_recv(socket, message, sizeof(message), 0);
        message[255] = '\0';  // ensure null-termination
        printf("Received request: %s\n", message);

        // simulate some work (optional)
        sleep(1);

        // send a reply back to the client
        const char *reply = "Hello from server";
        zmq_send(socket, reply, strlen(reply), 0);
    }

    // close the socket and terminate the context
    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}
