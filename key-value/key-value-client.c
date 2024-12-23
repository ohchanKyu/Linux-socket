#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5000
#define BUFFER_SIZE 1024

int main() {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket failed");
        return -1;
    }

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    printf("Connected to Key-Value Store Server\n");

    char buffer[BUFFER_SIZE];

    while (1) {
        printf("Enter command (SET <key> <value> or GET <key>): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        if (strncmp(buffer, "exit", 4) == 0) break;

        write(sock, buffer, strlen(buffer));

        int read_bytes = read(sock, buffer, BUFFER_SIZE - 1);
        if (read_bytes <= 0) break;
        buffer[read_bytes] = '\0';

        printf("Server response: %s", buffer);
    }

    close(sock);

    printf("Disconnected from server\n");

    return 0;
}
