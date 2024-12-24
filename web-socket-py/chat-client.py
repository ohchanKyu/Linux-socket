#!/usr/bin/python3

import socket
import threading

# Server configuration
HOST = 'localhost'
PORT = 8765

# Function to handle receiving messages from the server
def receive_messages(client_socket):
    while True:
        try:
            message = client_socket.recv(1024).decode('utf-8')
            if message:
                print(f"\rReceived from server: {message}")
        except Exception as e:
            print(f"Connection closed by server: {e}")
            break

# Function to handle sending messages to the server
def send_messages(client_socket):
    while True:
        try:
            message = input("Enter message: ")
            client_socket.send(message.encode('utf-8'))
        except Exception as e:
            print(f"Failed to send message: {e}")
            break

# Main client function
def start_client():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client_socket.connect((HOST, PORT))
        print("Connected to the server.")

        # Start threads for sending and receiving messages
        receive_thread = threading.Thread(target=receive_messages, args=(client_socket,))
        send_thread = threading.Thread(target=send_messages, args=(client_socket,))

        receive_thread.start()
        send_thread.start()

        # Wait for threads to complete
        receive_thread.join()
        send_thread.join()
    finally:
        client_socket.close()
        print("Disconnected from the server.")

start_client()
