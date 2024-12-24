#!/usr/bin/python3

import socket

def echo_client(host='127.0.0.1', port=5000):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((host, port))
        print("Connected to server")

        while True:
            message = input("Enter message to send (or 'exit' to quit): ")
            if message.lower() == 'exit':
                break
            sock.sendall(message.encode())

            data = sock.recv(1024)
            print("Received from server:", data.decode())

    print("Connection closed")

echo_client()
