#include <stdio.h>
#include <string.h>
#include <zmq.h>

int main() {
    // create a ZeroMQ context
    void *context = zmq_ctx_new();

    // create a REQ (request) socket and connect to the server
    void *socket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(socket, "tcp://localhost:5000");

    printf("Connected to the server on port 5000\n");

    char request_message[256];

    while (1) {
        // get input from the user to send as a request
        printf("Enter your message (or 'exit' to quit): ");
        fgets(request_message, sizeof(request_message), stdin);

        // remove newline character from the input
        size_t len = strlen(request_message);
        if (len > 0 && request_message[len - 1] == '\n') {
            request_message[len - 1] = '\0';
        }

        // check for the exit condition
        if (strcmp(request_message, "exit") == 0) {
            break;
        }

        // send the request to the server
        printf("Sending request: %s\n", request_message);
        zmq_send(socket, request_message, strlen(request_message), 0);

        // wait for the reply from the server
        char reply_message[256];
        zmq_recv(socket, reply_message, sizeof(reply_message), 0);
        reply_message[255] = '\0'; // ensure null-termination
        printf("Received reply: %s\n", reply_message);
    }

    printf("Disconnected from the server\n");

    // close the socket and terminate the context
    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}
