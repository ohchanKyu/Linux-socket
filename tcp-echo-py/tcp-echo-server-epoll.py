#!/usr/bin/python3

import socket
import select

def epoll_server(host='0.0.0.0', port=5000):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen()
    server_socket.setblocking(0)

    print("Listening on port", port)

    epoll = select.epoll()
    epoll.register(server_socket.fileno(), select.EPOLLIN)
    connections = {}

    try:
        while True:
            events = epoll.poll()
            for fileno, event in events:
                if fileno == server_socket.fileno():
                    client_socket, client_address = server_socket.accept()

                    client_socket.setblocking(0)
                    epoll.register(client_socket.fileno(), select.EPOLLIN)
                    connections[client_socket.fileno()] = client_socket

                    print("Accepted connection from", client_address)

                elif event & select.EPOLLIN:
                    client_socket = connections[fileno]

                    data = client_socket.recv(1024)
                    if not data:
                        epoll.unregister(fileno)
                        client_socket.close()
                        del connections[fileno]

                        print("Client disconnected")

                    else:
                        print("Received:", data.decode())
                        client_socket.sendall(data)

    finally:
        epoll.unregister(server_socket.fileno())
        epoll.close()
        server_socket.close()

# run the server
epoll_server()
