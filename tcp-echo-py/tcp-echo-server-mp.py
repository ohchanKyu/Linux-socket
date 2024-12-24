#!/usr/bin/python3

import socket
import multiprocessing

def handle_client(client_socket):
    with client_socket:
        while True:
            data = client_socket.recv(1024)
            if not data:
                break

            print("Received:", data.decode())

            client_socket.sendall(data)

def multiprocessing_server(host='0.0.0.0', port=12345):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen()

    print("Listening on port", port)

    try:
        while True:
            client_socket, addr = server_socket.accept()

            print("Accepted connection from", addr)

            process = multiprocessing.Process(target=handle_client, args=(client_socket,))
            process.start()

            client_socket.close()

    finally:
        server_socket.close()

# run the server
multiprocessing_server()
