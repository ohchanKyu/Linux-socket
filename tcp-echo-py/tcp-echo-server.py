#!/usr/bin/python3

import socket

def simple_echo_server(host='0.0.0.0', port=5000):
    # create a TCP socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen(1)

    print(f"Listening on {host}:{port}")

    # wait for a client connection
    client_socket, client_address = server_socket.accept()

    print(f"Accepted connection from {client_address}")

    with client_socket:
        while True:
            # receive data from the client
            data = client_socket.recv(1024)
            if not data:
                # no data received, client has disconnected
                print("Client disconnected")
                break

            # print and echo the received data
            print("Received:", data.decode())
            client_socket.sendall(data)  # Echo back the received data

    # close the server socket
    server_socket.close()
    print("Server shut down")

# run the server
simple_echo_server()
