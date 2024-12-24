#!/usr/bin/python3

import socket
import select

def select_server(host='0.0.0.0', port=12345):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen()

    print("Listening on port", port)

    sockets_list = [server_socket]
    clients = {}

    try:
        while True:
            read_sockets, _, _ = select.select(sockets_list, [], [])

            for notified_socket in read_sockets:
                if notified_socket == server_socket:
                    client_socket, client_address = server_socket.accept()

                    sockets_list.append(client_socket)
                    clients[client_socket] = client_address

                    print("Accepted connection from", client_address)

                else:
                    data = notified_socket.recv(1024)
                    if not data:
                        print("Connection closed from", clients[notified_socket])

                        sockets_list.remove(notified_socket)
                        del clients[notified_socket]
                        notified_socket.close()

                    else:
                        print("Received:", data.decode())
                        notified_socket.sendall(data)

    finally:
        server_socket.close()

# run the server
select_server()
