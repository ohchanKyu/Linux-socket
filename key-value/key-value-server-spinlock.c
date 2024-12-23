#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 5000
#define BUFFER_SIZE 1024

// node for key-value pair linked list
typedef struct Node {
    char key[BUFFER_SIZE];
    char value[BUFFER_SIZE];
    struct Node *next;
} Node;

// head of the linked list for key-value pairs
Node *head = NULL;

pthread_spinlock_t store_spinlock;

// function to set a key-value pair in the store
void set_key_value(const char *key, const char *value) {
    pthread_spin_lock(&store_spinlock);

    Node *current = head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            strncpy(current->value, value, BUFFER_SIZE);
            pthread_spin_unlock(&store_spinlock);
            return;
        }
        current = current->next;
    }

    Node *new_node = (Node *)malloc(sizeof(Node));
    strncpy(new_node->key, key, BUFFER_SIZE);
    strncpy(new_node->value, value, BUFFER_SIZE);
    new_node->next = head;
    head = new_node;

    pthread_spin_unlock(&store_spinlock);
}

// function to get a value for a given key
const char *get_value(const char *key) {
    pthread_spin_lock(&store_spinlock);

    Node *current = head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            pthread_spin_unlock(&store_spinlock);
            return current->value;
        }
        current = current->next;
    }

    pthread_spin_unlock(&store_spinlock);

    return "NOT FOUND";
}

// function to handle each client connection
void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];

    while (1) {
        int read_bytes = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (read_bytes <= 0) break;
        buffer[read_bytes] = '\0';

        char command[BUFFER_SIZE], key[BUFFER_SIZE], value[BUFFER_SIZE];
        int tokens = sscanf(buffer, "%s %s %[^\n]", command, key, value);

        if (tokens >= 2 && strcmp(command, "GET") == 0) {
            const char *result = get_value(key);
            snprintf(buffer, BUFFER_SIZE, "%s\n", result);
        } else if (tokens == 3 && strcmp(command, "SET") == 0) {
            set_key_value(key, value);
            snprintf(buffer, BUFFER_SIZE, "OK\n");
        } else {
            snprintf(buffer, BUFFER_SIZE, "ERROR: Invalid command\n");
        }

        write(client_fd, buffer, strlen(buffer));
    }

    close(client_fd);

    return NULL;
}

int main() {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        return -1;
    }

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        return -1;
    }

    printf("Listening on port %d\n", PORT);

    if (pthread_spin_init(&store_spinlock, 0) != 0) {
        perror("Spinlock init failed");
        close(server_fd);
        return -1;
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int *client_fd = malloc(sizeof(int));
        if ((*client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("Accept failed");
            free(client_fd);
            continue;
        }

        printf("Accepted connection from client\n");

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, client_fd) != 0) {
            perror("Thread creation failed");
            free(client_fd);
            continue;
        }

        pthread_detach(thread); 
    }

    pthread_spin_destroy(&store_spinlock);
    close(server_fd);

    return 0;
}
