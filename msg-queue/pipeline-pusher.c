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

    // create a PUSH (producer) socket and bind it to a port
    void *socket = zmq_socket(context, ZMQ_PUSH);
    zmq_bind(socket, "tcp://*:5000");

    printf("Producer started, sending tasks on port 5000...\n");

    for (int task_num = 1; task_num <= 10; task_num++) {  // Sending 10 tasks
        // generate a random "task" (a message with a simulated workload duration)
        int workload = rand() % 5 + 1;  // task duration between 1 to 5 seconds

        // send the task to workers
        char message[256];
        snprintf(message, sizeof(message), "Task %d: %d seconds", task_num, workload);
        printf("Sending: %s\n", message);
        zmq_send(socket, message, strlen(message), 0);

        // simulate time between task submissions
        sleep(1);
    }

    printf("All tasks have been sent\n");

    // close the socket and terminate the context
    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}
