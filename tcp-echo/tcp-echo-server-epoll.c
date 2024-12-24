#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#define PORT 5000
#define BUFFER_SIZE 1024

#define MAX_EVENTS 10
#define MAX_CLIENTS 100

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
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_fd);
        return -1;
    }

    printf("Listening on port %d...\n", PORT);

    int epoll_fd;
    struct epoll_event event, events[MAX_EVENTS];

    // create an epoll instance
    if ((epoll_fd = epoll_create1(0)) < 0) {
        perror("epoll_create1 failed");
        close(server_fd);
        return -1;
    }

    // add the server socket to the epoll instance
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
        perror("epoll_ctl failed");
        close(server_fd);
        close(epoll_fd);
        return -1;
    }

    while (1) {
        // wait for events
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events < 0) {
            perror("epoll_wait failed");
            break;
        }

        char buffer[BUFFER_SIZE];

        // process each event
        for (int i = 0; i < num_events; i++) {
            int event_fd = events[i].data.fd;

            // if the event is on the server socket, it means a new client connection
            if (event_fd == server_fd) {
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_fd < 0) {
                    perror("Accept failed");
                    continue;
                }
                printf("New client connected, socket fd is %d\n", client_fd);

                // add the new client socket to the epoll instance
                event.events = EPOLLIN | EPOLLET; // Edge-triggered
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
                    perror("epoll_ctl failed");
                    close(client_fd);
                    continue;
                }
            } else {
                // handle client data
                int read_bytes = read(event_fd, buffer, BUFFER_SIZE);
                if (read_bytes == 0) {
                    // client disconnected
                    printf("Client disconnected (socket fd: %d)\n", event_fd);
                    close(event_fd);
                    // remove from epoll instance
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event_fd, NULL);
                } else if (read_bytes > 0) {
                    // echo the message back to the client
                    buffer[read_bytes] = '\0';
                    printf("Received (%d bytes) from client %d: %s", read_bytes, event_fd, buffer);
                    send(event_fd, buffer, read_bytes, 0);
                } else {
                    // error handling
                    perror("Read error");
                    close(event_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, event_fd, NULL);
                }
            }
        }
    }

    // close the server socket and epoll instance
    close(server_fd);
    close(epoll_fd);

    return 0;
}
