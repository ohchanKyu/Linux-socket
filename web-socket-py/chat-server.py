#!/usr/bin/python3

import socket
import threading

# Server configuration
HOST = 'localhost'
PORT = 8765

# List to keep track of connected clients
clients = []

# Function to handle each client connection
def handle_client(client_socket, client_address):
    print(f"New connection from {client_address}")
    clients.append(client_socket)
    try:
        while True:
            message = client_socket.recv(1024).decode('utf-8')
            if not message:
                break
            print(f"Received message from {client_address}: {message}")
            broadcast(message, client_socket)
    finally:
        print(f"Client {client_address} disconnected")
        clients.remove(client_socket)
        client_socket.close()

# Function to broadcast a message to all clients except the sender
def broadcast(message, sender_socket):
    for client in clients:
        if client != sender_socket:
            try:
                client.send(message.encode('utf-8'))
            except Exception as e:
                print(f"Failed to send message to a client: {e}")

# Main server function
def start_server():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((HOST, PORT))
    server_socket.listen(5)
    print(f"Server started on {HOST}:{PORT}")

    while True:
        client_socket, client_address = server_socket.accept()
        client_thread = threading.Thread(target=handle_client, args=(client_socket, client_address))
        client_thread.start()

start_server()
