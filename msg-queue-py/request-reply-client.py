#!/usr/bin/python3

import zmq

def zmq_client():
    # create a ZeroMQ context
    context = zmq.Context()

    # create a REQ (request) socket and connect to the server
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5000")

    print("Connected to the server on port 5000")

    while True:
        # get input from the user to send as a request
        request_message = input("Enter your message (or 'exit' to quit): ")
        if request_message.lower() == 'exit':
            break

        # send the request to the server
        print(f"Sending request: {request_message}")
        socket.send_string(request_message)

        # wait for the reply from the server
        reply = socket.recv_string()
        print(f"Received reply: {reply}")

    print("Disconnected from the server")

# run the client
zmq_client()
