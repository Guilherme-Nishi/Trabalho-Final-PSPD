#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    int powmin, powmax;

    // Solicitar ao usuário os valores de POWMIN e POWMAX
    printf("Digite o valor de POWMIN: ");
    scanf("%d", &powmin);
    printf("Digite o valor de POWMAX: ");
    scanf("%d", &powmax);

    // Criar o socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nErro na criação do socket\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converter endereço IP para formato binário
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nEndereço inválido ou não suportado\n");
        return -1;
    }

    // Conectar ao servidor
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nFalha na conexão\n");
        return -1;
    }

    // Preparar a mensagem com POWMIN e POWMAX
    snprintf(buffer, sizeof(buffer), "%d %d", powmin, powmax);

    // Enviar a mensagem para o servidor
    send(sock, buffer, strlen(buffer), 0);
    printf("Parâmetros enviados: POWMIN=%d, POWMAX=%d\n", powmin, powmax);

    // Fechar o socket
    close(sock);

    return 0;
}
