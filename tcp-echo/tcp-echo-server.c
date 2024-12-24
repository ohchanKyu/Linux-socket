#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5000
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
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

    printf("waiting for a connection...\n");

    // accept a client connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
        perror("Accept failed");
        close(server_fd);
        return -1;
    }

    printf("accepted a new connection\n");

    int read_bytes;
    char buffer[BUFFER_SIZE];

    // receive and echo back data
    while ((read_bytes = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
        buffer[read_bytes] = '\0';
        printf("Received: %s", buffer);

        // echo the received message back to the client
        send(client_fd, buffer, read_bytes, 0);
    }

    // close the connection
    close(client_fd);
    close(server_fd);

    return 0;
}
