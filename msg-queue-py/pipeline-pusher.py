#!/usr/bin/python3

import zmq
import time
import random

def zmq_push_producer():
    # create a ZeroMQ context
    context = zmq.Context()

    # create a PUSH (producer) socket and bind it to a port
    socket = context.socket(zmq.PUSH)
    socket.bind("tcp://*:5000")

    print("Producer started, sending tasks on port 5000...")

    for task_num in range(1, 11):  # Sending 10 tasks
        # generate a random "task" (a message with a simulated workload duration)
        workload = random.randint(1, 5)  # task duration between 1 to 5 seconds
        message = f"Task {task_num}: {workload} seconds"

        # send the task to workers
        print(f"Sending: {message}")
        socket.send_string(message)

        # simulate time between task submissions
        time.sleep(1)

    print("All tasks have been sent")

# run the producer
zmq_push_producer()
