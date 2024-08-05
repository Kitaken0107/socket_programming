#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define PORT 8080
#define MAX_PENDING 10

int server_fd;
int keep_running = 1;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *client_socket) {
    int sock = *(int *)client_socket;
    free(client_socket);

    // Process client request
    pthread_mutex_lock(&lock);
    static int connection_count = 0;
    int client_id = ++connection_count;
    pthread_mutex_unlock(&lock);

    char buffer[1024] = {0};
    sprintf(buffer, "%d", client_id);

    send(sock, buffer, strlen(buffer), 0);
    printf("Assigned ID %d to client\n", client_id);

    close(sock);
    return NULL;
}

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nSIGINT received. Shutting down...\n");
        keep_running = 0;
        // Close the server socket to stop accepting new connections
        close(server_fd);
    }
}

int main() {
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Register signal handler
    signal(SIGINT, signal_handler);

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

    while (keep_running) {
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            if (!keep_running) {
                break;  // Exit the loop if we are shutting down
            }
            perror("accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        pthread_t thread_id;
        int *client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        if (pthread_create(&thread_id, NULL, handle_client, client_socket) != 0) {
            perror("pthread_create failed");
            close(new_socket);
            free(client_socket);
        } else {
            pthread_detach(thread_id);
        }
    }

    // Clean up remaining connections
    printf("Server has stopped accepting new connections. Cleaning up...\n");

    // Wait for all threads to finish
    // In a more complex implementation, you might want to join threads or handle cleanup here

    pthread_mutex_destroy(&lock);
    return 0;
}
