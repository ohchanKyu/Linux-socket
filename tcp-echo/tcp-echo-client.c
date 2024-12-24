#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5000
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;

    // create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    // connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    char send_buffer[BUFFER_SIZE];
    char recv_buffer[BUFFER_SIZE];

    while (1) {
        printf("Enter a message: ");
        fgets(send_buffer, BUFFER_SIZE, stdin);

        // send message to the server
        send(sock, send_buffer, strlen(send_buffer), 0);

        // receive echo message from the server
        int read_bytes = read(sock, recv_buffer, BUFFER_SIZE);
        recv_buffer[read_bytes] = '\0';

        // print out the received message
        printf("Received (%d bytes): %s", read_bytes, recv_buffer);
    }

    // close the socket
    close(sock);

    return 0;
}
