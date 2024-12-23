#!/usr/bin/python3
import socket

def key_value_client(host='127.0.0.1', port=5000):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((host, port))
        print("Connected to Key-Value Store Server")

        while True:
            command = input("Enter command (SET <key> <value> or GET <key>): ").strip()
            if command.lower() == 'exit':
                break
            sock.sendall(command.encode())
            response = sock.recv(1024).decode()

            print("Server response:", response)

    print("Disconnected from server")

key_value_client()
