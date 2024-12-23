#!/usr/bin/python3

class ReadWriteLock:
    def __init__(self):
        self._read_ready = threading.Lock()
        self._readers = 0
        self._resource_lock = threading.Lock()

    def acquire_read(self):
        with self._read_ready:
            self._readers += 1
            if self._readers == 1:
                self._resource_lock.acquire()

    def release_read(self):
        with self._read_ready:
            self._readers -= 1
            if self._readers == 0:
                self._resource_lock.release()

    def acquire_write(self):
        self._resource_lock.acquire()

    def release_write(self):
        self._resource_lock.release()

store = {}
store_rwlock = ReadWriteLock()

def handle_client_rwlock(client_socket):
    with client_socket:
        while True:
            data = client_socket.recv(1024).decode()
            if not data:
                break  
            parts = data.strip().split(" ", 2)
            command = parts[0].upper()

            if command == "SET" and len(parts) == 3:
                key, value = parts[1], parts[2]
                store_rwlock.acquire_write()
                store[key] = value
                store_rwlock.release_write()
                response = "OK\n"
            elif command == "GET" and len(parts) == 2:
                key = parts[1]
                store_rwlock.acquire_read()
                response = store.get(key, "NOT FOUND") + "\n"
                store_rwlock.release_read()
            else:
                response = "ERROR: Invalid command\n"

            client_socket.sendall(response.encode())

def key_value_server_rwlock(host='0.0.0.0', port=5000):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen()

    print("Listening on", host, port)

    try:
        while True:
            client_socket, addr = server_socket.accept()

            print("Accepted connection from", addr)
            thread = threading.Thread(target=handle_client_rwlock, args=(client_socket,))
            thread.start()
    finally:
        server_socket.close()

key_value_server_rwlock()
