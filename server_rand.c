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

int compare_ints(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

void *handle_client(void *client_socket) {
    int sock = *(int *)client_socket;
    free(client_socket);

    pthread_mutex_lock(&lock);
    int client_id = ++connection_count;
    pthread_mutex_unlock(&lock);

    char buffer[1024] = {0};
    sprintf(buffer, "%d", client_id);

    send(sock, buffer, strlen(buffer), 0);
    printf("Assigned ID %d to client\n", client_id);

    // クライアントからランダムな整数を受け取る
    int num_integers = client_id;
    int *received_ints = malloc(num_integers * sizeof(int));
    int valread = read(sock, received_ints, num_integers * sizeof(int));

    if (valread != num_integers * sizeof(int)) {
        perror("Error reading integers from client");
        close(sock);
        free(received_ints);
        return NULL;
    }

    printf("Received integers from client: ");
    for (int i = 0; i < num_integers; ++i) {
        printf("%d ", received_ints[i]);
    }
    printf("\n");

    // 受け取った整数をソート
    qsort(received_ints, num_integers, sizeof(int), compare_ints);

    // ソートした整数をクライアントに返す
    send(sock, received_ints, num_integers * sizeof(int), 0);
    printf("Sent sorted integers back to client\n");

    free(received_ints);
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
