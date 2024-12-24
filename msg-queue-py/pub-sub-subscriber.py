#!/usr/bin/python3

import zmq

def zmq_subscriber(topic="sports"):
    # Create a ZeroMQ context
    context = zmq.Context()

    # Create a SUB (subscriber) socket and connect to the publisher
    socket = context.socket(zmq.SUB)
    socket.connect("tcp://localhost:5556")
    print(f"Connected to publisher on port 5556, subscribing to '{topic}'")

    # Subscribe to the specified topic
    socket.setsockopt_string(zmq.SUBSCRIBE, topic)

    while True:
        # Receive messages that match the subscribed topic
        message = socket.recv_string()
        print(f"Received: {message}")

# run the subscriber
zmq_subscriber("sports")  # Change "sports" to "news" or "weather" to subscribe to different topics
