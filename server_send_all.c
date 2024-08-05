#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_PENDING 10

int connection_count = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *client_socket) {
    int sock = *(int *)client_socket;
    free(client_socket);

    pthread_mutex_lock(&lock);
    int client_id = ++connection_count;
    pthread_mutex_unlock(&lock);

    char buffer[1024] = {0};
    sprintf(buffer, "%d", client_id);
    size_t total_sent = 0;
    size_t to_send = strlen(buffer);

    while (total_sent < to_send) {
        ssize_t sent = send(sock, buffer + total_sent, to_send - total_sent, 0);
        if (sent == -1) {
            perror("send failed");
            close(sock);
            return NULL;
        }
        total_sent += sent;
    }

    printf("Assigned ID %d to client\n", client_id);

    close(sock);
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_PENDING) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        int *client_socket = malloc(sizeof(int));
        *client_socket = new_socket;
        pthread_t thread_id;

        if (pthread_create(&thread_id, NULL, handle_client, client_socket) != 0) {
            perror("pthread_create failed");
            close(new_socket);
            free(client_socket);
        }

        pthread_detach(thread_id);
    }

    close(server_fd);
    pthread_mutex_destroy(&lock);
    return 0;
}
