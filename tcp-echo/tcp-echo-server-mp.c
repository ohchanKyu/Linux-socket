#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT 5000
#define BUFFER_SIZE 1024

void handle_client(int client_fd) {
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

    return;
}

void sigchld_handler(int sig) {
    // wait for all dead processes to prevent zombies
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // set up SIGCHLD handler to clean up zombie processes
    signal(SIGCHLD, sigchld_handler);

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
        // accept a client connection
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Accepted a new connection\n");

        // create a new process to handle the client
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(client_fd);
            continue;
        } else if (pid == 0) { // child
            // close the server socket in the child process
            close(server_fd);
            // handle the client connection
            handle_client(client_fd);
        } else { // parent
            // close the client socket in the parent process
            close(client_fd);
        }
    }

    // close the server socket
    close(server_fd);

    return 0;
}
