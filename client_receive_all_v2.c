#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024];
    size_t total_received = 0;
    ssize_t bytes_received;

    // ソケットの作成
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    // サーバーのアドレス設定
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        return -1;
    }

    // サーバーへの接続
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // バッファのゼロクリア
    memset(buffer, 0, sizeof(buffer));

    // データの受信
    while ((bytes_received = read(sock, buffer + total_received, sizeof(buffer) - total_received - 1)) > 0) {
        total_received += bytes_received;
    }

    // エラーチェック
    if (bytes_received < 0) {
        perror("read failed");
        close(sock);
        return -1;
    }

    // 受信データの表示
    printf("Server response: %s\n", buffer);

    // ソケットの閉鎖
    close(sock);
    return 0;
}
