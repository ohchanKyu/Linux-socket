#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>

#define PORT 5000
#define BUFFER_SIZE 1024

#define MAX_CLIENTS 30

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // create the server socket
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
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        return -1;
    }

    printf("Listening on port %d...\n", PORT);

    int client_sockets[MAX_CLIENTS] = {0};
    int max_fd = server_fd; // initialize the maximum file descriptor value
    fd_set read_fds; // set of socket descriptors

    char buffer[BUFFER_SIZE];

    while (1) {
        // clear the socket set
        FD_ZERO(&read_fds);

        // add the server socket to the set
        FD_SET(server_fd, &read_fds);

        // add client sockets to the set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &read_fds);
            }

            if (client_sockets[i] > max_fd) {
                max_fd = client_sockets[i];
            }
        }

        // wait for an activity on any of the sockets, with no timeout
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("Select error");
            return -1;
        }

        // check if there is an incoming connection on the server socket
        if (FD_ISSET(server_fd, &read_fds)) {
            if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
                perror("Accept failed");
                return -1;
            }

            printf("New client connected (socket fd: %d)\n", client_fd);

            // add new client socket to the array
            int added = 0;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_fd;
                    added = 1;
                    printf("Added client socket to list at index %d\n", i);
                    break;
                }
            }

            if (!added) {
                printf("Maximum clients reached, connection rejected\n");
                close(client_fd);
            }
        }

        // check for activity on client sockets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            client_fd = client_sockets[i];

            if (FD_ISSET(client_fd, &read_fds)) {
                // read the message from the client
                int read_bytes = read(client_fd, buffer, BUFFER_SIZE);
                if (read_bytes == 0) {
                    // client disconnected
                    printf("Client disconnected (socket fd: %d)\n", client_fd);
                    close(client_fd);
                    client_sockets[i] = 0;
                } else {
                    // echo the message back to the client
                    buffer[read_bytes] = '\0';
                    printf("Received (%d bytes): %s", read_bytes, buffer);
                    send(client_fd, buffer, read_bytes, 0);
                }
            }
        }
    }

    // close the server socket
    close(server_fd);

    return 0;
}
