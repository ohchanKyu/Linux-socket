#!/usr/bin/python3

import zmq
import time

def zmq_server():
    # create a ZeroMQ context
    context = zmq.Context()

    # create a REP (reply) socket and bind it to a port
    socket = context.socket(zmq.REP)
    socket.bind("tcp://*:5000")

    print("Listening on port 5000...")

    while True:
        # wait for a request from the client
        message = socket.recv_string()
        print(f"Received request: {message}")

        # simulate some work (optional)
        time.sleep(1)

        # send a reply back to the client
        socket.send_string("Hello from server")

# run the server
zmq_server()
