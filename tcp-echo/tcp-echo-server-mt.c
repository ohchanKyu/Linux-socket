#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 5000
#define BUFFER_SIZE 1024

void *handle_client(void *client_socket) {
    int client_fd = *(int *)client_socket;

    // free the allocated memory for the socket descriptor
    free(client_socket);

    int read_bytes;
    char buffer[BUFFER_SIZE];

    // receive and echo back data
    while ((read_bytes = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
        buffer[read_bytes] = '\0';
        printf("Received (%d bytes): %s", read_bytes, buffer);

        // echo the received message back to the client
        send(client_fd, buffer, read_bytes, 0);
    }

    // close the client connection
    close(client_fd);
    printf("Client disconnected\n");

    return NULL;
}

int main() {
    pthread_t thread_id;
    int server_fd, *client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        return -1;
    }

    // configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // bind the socket to the port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return -1;
    }

    // start listening for incoming connections
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        return -1;
    }

    printf("Listening on port %d...\n", PORT);

    while (1) {
        // allocate memory for client socket
        client_fd = malloc(sizeof(int));
        if (!client_fd) {
            perror("Failed to allocate memory for client socket");
            continue;
        }

        // accept a client connection
        if ((*client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("Accept failed");
            free(client_fd);
            continue;
        }

        printf("Accepted a new connection\n");

        // create a new thread to handle the client
        if (pthread_create(&thread_id, NULL, handle_client, client_fd) != 0) {
            perror("Failed to create thread");
            free(client_fd);
            continue;
        }

        // detach the thread so it cleans up automatically after finishing
        pthread_detach(thread_id);
    }

    // close the server socket
    close(server_fd);

    return 0;
}
