#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void send_to_mpi(int powmin, int powmax) {
    char command[256];
    snprintf(command, sizeof(command), "mpirun -np 4 ./jogodavidampi %d %d", powmin, powmax);
    printf("Executando comando: %s\n", command);
    int ret = system(command);
    if (ret == -1) {
        perror("Falha ao executar comando MPI");
    }
}

void send_to_omp(int powmin, int powmax) {
    char command[256];
    snprintf(command, sizeof(command), "./jogodavidaomp %d %d", powmin, powmax);
    printf("Executando comando: %s\n", command);
    int ret = system(command);
    if (ret == -1) {
        perror("Falha ao executar comando OpenMP");
    }
}

// Função para lidar com cada cliente
void *handle_client(void *client_socket_ptr) {
    int client_socket = *(int *)client_socket_ptr;
    char buffer[BUFFER_SIZE];
    int read_size;

    // Recebe os dados do cliente
    if ((read_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';  // Garantir que a string está terminada

        // Parse dos parâmetros POWMIN e POWMAX
        int powmin, powmax;
        if (sscanf(buffer, "%d %d", &powmin, &powmax) == 2) {
            printf("Recebido POWMIN: %d, POWMAX: %d\n", powmin, powmax);

            // Executa os engines
            send_to_mpi(powmin, powmax);
            send_to_omp(powmin, powmax);

            char *response = "Parâmetros recebidos e processados.";
            send(client_socket, response, strlen(response), 0);
        } else {
            printf("Dados inválidos recebidos\n");
        }
    }

    // Fecha a conexão com o cliente
    close(client_socket);
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id;

    // Cria o socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configura o endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Faz o bind do socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Escuta por conexões
    if (listen(server_socket, 3) < 0) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Servidor ouvindo na porta %d...\n", PORT);

    // Aceita e lida com conexões dos clientes
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) >= 0) {
        printf("Cliente conectado\n");

        // Cria uma nova thread para lidar com o cliente
        if (pthread_create(&thread_id, NULL, handle_client, &client_socket) != 0) {
            perror("pthread_create");
        }

        // Detach a thread
        pthread_detach(thread_id);
    }

    // Fecha o socket do servidor
    close(server_socket);
    return 0;
}
