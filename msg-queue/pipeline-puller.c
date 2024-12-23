#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

int main() {
    // create a ZeroMQ context
    void *context = zmq_ctx_new();

    // create a PULL (worker) socket and connect to the producer
    void *socket = zmq_socket(context, ZMQ_PULL);
    zmq_connect(socket, "tcp://localhost:5000");

    printf("Worker connected to producer on port 5000\n");

    while (1) {
        // receive a task from the producer
        char message[256];
        zmq_recv(socket, message, sizeof(message) - 1, 0);
        message[255] = '\0'; // ensure null-termination
        printf("Received: %s\n", message);

        // parse the workload time from the message
        int workload_time;
        sscanf(message, "Task: %d", &workload_time);

        // simulate task processing
        sleep(workload_time);
        printf("Task completed: %s\n", message);
    }

    // close the socket and terminate the context
    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}
