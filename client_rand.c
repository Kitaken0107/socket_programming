#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    valread = read(sock, buffer, 1024);
    int num_integers = atoi(buffer);  // サーバーからの数値を取得
    printf("Received number from server: %d\n", num_integers);

    // ランダムな整数を生成
    srand(time(0));
    int *random_integers = malloc(num_integers * sizeof(int));
    for (int i = 0; i < num_integers; ++i) {
        random_integers[i] = rand() % 1024;  // 0から1023までのランダムな整数を生成
    }

    // ランダムな整数をサーバーに送信
    send(sock, random_integers, num_integers * sizeof(int), 0);

    // サーバーからソートされた整数を受け取る
    int *sorted_integers = malloc(num_integers * sizeof(int));
    valread = read(sock, sorted_integers, num_integers * sizeof(int));
    if (valread != num_integers * sizeof(int)) {
        perror("Error reading sorted integers from server");
        close(sock);
        free(random_integers);
        free(sorted_integers);
        return -1;
    }

    printf("Received sorted integers from server: ");
    for (int i = 0; i < num_integers; ++i) {
        printf("%d ", sorted_integers[i]);
    }
    printf("\n");

    free(random_integers);
    free(sorted_integers);
    close(sock);
    return 0;
}
