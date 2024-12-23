#!/usr/bin/python3

import socket
import threading

store = {}

store_lock = threading.Lock()

def handle_client(client_socket):
    with client_socket:
        while True:
            data = client_socket.recv(1024).decode()
            if not data:
                break  
            parts = data.strip().split(" ", 2)
            command = parts[0].upper()

            if command == "SET" and len(parts) == 3:
                key, value = parts[1], parts[2]
                with store_lock:
                    store[key] = value
                response = "OK\n"
            elif command == "GET" and len(parts) == 2:
                key = parts[1]
                with store_lock:
                    response = store.get(key, "NOT FOUND") + "\n"
            else:
                response = "ERROR: Invalid command\n"

            client_socket.sendall(response.encode())

def key_value_server_mutex(host='0.0.0.0', port=5000):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen()

    print("Listening on", host, port)

    try:
        while True:
            client_socket, addr = server_socket.accept()

            print("Accepted connection from", addr)
            thread = threading.Thread(target=handle_client, args=(client_socket,))
            thread.start()
    finally:
        server_socket.close()

key_value_server_mutex()
