#!/usr/bin/python3

import socket
import threading

def handle_client(client_socket):
    with client_socket:
        while True:
            data = client_socket.recv(1024)
            if not data:
                break

            print("Received:", data.decode())

            client_socket.sendall(data)

def multithreading_server(host='0.0.0.0', port=12345):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen()

    print("Listening on port", port)

    try:
        while True:
            client_socket, addr = server_socket.accept()

            print("Accepted connection from", addr)

            thread = threading.Thread(target=handle_client, args=(client_socket,))
            thread.start()

    finally:
        server_socket.close()

# run the server
multithreading_server()
