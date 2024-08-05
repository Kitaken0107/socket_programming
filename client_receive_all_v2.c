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

    // �\�P�b�g�̍쐬
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    // �T�[�o�[�̃A�h���X�ݒ�
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        return -1;
    }

    // �T�[�o�[�ւ̐ڑ�
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // �o�b�t�@�̃[���N���A
    memset(buffer, 0, sizeof(buffer));

    // �f�[�^�̎�M
    while ((bytes_received = read(sock, buffer + total_received, sizeof(buffer) - total_received - 1)) > 0) {
        total_received += bytes_received;
    }

    // �G���[�`�F�b�N
    if (bytes_received < 0) {
        perror("read failed");
        close(sock);
        return -1;
    }

    // ��M�f�[�^�̕\��
    printf("Server response: %s\n", buffer);

    // �\�P�b�g�̕�
    close(sock);
    return 0;
}
