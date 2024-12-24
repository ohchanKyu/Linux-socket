#!/usr/bin/python3

import zmq
import time
import random

def zmq_publisher():
    # create a ZeroMQ context
    context = zmq.Context()

    # create a PUB (publisher) socket and bind it to a port
    socket = context.socket(zmq.PUB)
    socket.bind("tcp://*:5000")

    print("Publisher started, broadcasting on port 5000...")

    while True:
        # generate a random topic and message
        topic = random.choice(["sports", "news", "weather"])
        message = f"{topic} - Message at {time.ctime()}"

        # publish the message
        print(f"Publishing: {message}")
        socket.send_string(f"{topic} {message}")

        # wait for a second before sending the next message
        time.sleep(1)

# run the publisher
zmq_publisher()
