#!/usr/bin/python3

import time

class Spinlock:
    def __init__(self):
        self._lock = threading.Lock()

    def acquire(self):
        while not self._lock.acquire(blocking=False):
            time.sleep(0.001) 

    def release(self):
        self._lock.release()

store = {}
store_spinlock = Spinlock()

def handle_client_spinlock(client_socket):
    with client_socket:
        while True:
            data = client_socket.recv(1024).decode()
            if not data:
                break 

            parts = data.strip().split(" ", 2)
            command = parts[0].upper()

            if command == "SET" and len(parts) == 3:
                key, value = parts[1], parts[2]
                store_spinlock.acquire()
                store[key] = value
                store_spinlock.release()
                response = "OK\n"
            elif command == "GET" and len(parts) == 2:
                key = parts[1]
                store_spinlock.acquire()
                response = store.get(key, "NOT FOUND") + "\n"
                store_spinlock.release()
            else:
                response = "ERROR: Invalid command\n"

            client_socket.sendall(response.encode())

def key_value_server_spinlock(host='0.0.0.0', port=5000):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen()

    print("Listening on", host, port)

    try:
        while True:
            client_socket, addr = server_socket.accept()
            print("Accepted connection from", addr)

            thread = threading.Thread(target=handle_client_spinlock, args=(client_socket,))
            thread.start()

    finally:
        server_socket.close()

key_value_server_spinlock()
