#!/usr/bin/python3

import zmq
import time

def zmq_pull_worker():
    # create a ZeroMQ context
    context = zmq.Context()

    # create a PULL (worker) socket and connect to the producer
    socket = context.socket(zmq.PULL)
    socket.connect("tcp://localhost:5000")

    print("Worker connected to producer on port 5000")

    while True:
        # receive a task from the producer
        message = socket.recv_string()
        print(f"Received: {message}")

        # simulate task processing
        workload_time = int(message.split(": ")[1].split()[0])
        time.sleep(workload_time)
        print(f"Task completed: {message}")

# run the worker
zmq_pull_worker()
