#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

// Função para receber parâmetros do servidor de sockets
void receive_params_from_socket(int *powmin, int *powmax) {
    int sock;
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE];

    // Configurar o socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }
    server.sin_addr.s_addr = inet_addr("socket-server-service");  // Nome do serviço do servidor de sockets
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);  // Porta do servidor de sockets

    // Conectar ao servidor de sockets
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Falha ao conectar ao servidor de sockets");
        exit(EXIT_FAILURE);
    }

    // Receber a mensagem do servidor de sockets
    int read_size = recv(sock, buffer, sizeof(buffer), 0);
    if (read_size < 0) {
        perror("Falha ao receber dados");
        exit(EXIT_FAILURE);
    }

    buffer[read_size] = '\0';  // Garantir que a string está terminada

    // Parse dos parâmetros POWMIN e POWMAX
    if (sscanf(buffer, "%d %d", powmin, powmax) != 2) {
        fprintf(stderr, "Dados inválidos recebidos\n");
        exit(EXIT_FAILURE);
    }

    close(sock);
}

#define ind2d(i,j) ((i)*(tam+2)+(j))

double wall_time(void) {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return(tv.tv_sec + tv.tv_usec/1000000.0);
}

void UmaVida(int* tabulIn, int* tabulOut, int tam, int start, int end) {
    int i, j, vizviv;
    for (i = start; i <= end; i++) {
        for (j = 1; j <= tam; j++) {
            vizviv =  tabulIn[ind2d(i-1,j-1)] + tabulIn[ind2d(i-1,j )] +
                      tabulIn[ind2d(i-1,j+1)] + tabulIn[ind2d(i  ,j-1)] +
                      tabulIn[ind2d(i  ,j+1)] + tabulIn[ind2d(i+1,j-1)] +
                      tabulIn[ind2d(i+1,j  )] + tabulIn[ind2d(i+1,j+1)];
            if (tabulIn[ind2d(i,j)] && vizviv < 2)
                tabulOut[ind2d(i,j)] = 0;
            else if (tabulIn[ind2d(i,j)] && vizviv > 3)
                tabulOut[ind2d(i,j)] = 0;
            else if (!tabulIn[ind2d(i,j)] && vizviv == 3)
                tabulOut[ind2d(i,j)] = 1;
            else
                tabulOut[ind2d(i,j)] = tabulIn[ind2d(i,j)];
        }
    }
}

void DumpTabul(int * tabul, int tam, int first, int last, char* msg) {
    int i, ij;
    printf("%s; Dump posições [%d:%d, %d:%d] de tabuleiro %d x %d\n", msg, first, last, first, last, tam, tam);
    for (i = first; i <= last; i++) printf("="); printf("=\n");
    for (i = ind2d(first,0); i <= ind2d(last,0); i += ind2d(1,0)) {
        for (ij = i + first; ij <= i + last; ij++)
            printf("%c", tabul[ij] ? 'X' : '.');
        printf("\n");
    }
    for (i = first; i <= last; i++) printf("="); printf("=\n");
}

void InitTabul(int* tabulIn, int* tabulOut, int tam) {
    int ij;
    for (ij = 0; ij < (tam+2)*(tam+2); ij++) {
        tabulIn[ij] = 0;
        tabulOut[ij] = 0;
    }
    tabulIn[ind2d(1,2)] = 1; 
    tabulIn[ind2d(2,3)] = 1;
    tabulIn[ind2d(3,1)] = 1; 
    tabulIn[ind2d(3,2)] = 1;
    tabulIn[ind2d(3,3)] = 1;
}

int Correto(int* tabul, int tam) {
    int ij, cnt = 0;
    for (ij = 0; ij < (tam+2)*(tam+2); ij++)
        cnt += tabul[ij];
    return (cnt == 5 && tabul[ind2d(tam-2,tam-1)] &&
            tabul[ind2d(tam-1,tam  )] && tabul[ind2d(tam  ,tam-2)] &&
            tabul[ind2d(tam  ,tam-1)] && tabul[ind2d(tam  ,tam  )]);
}

int main(int argc, char** argv) {
    int powmin, powmax, i, tam, *tabulIn, *tabulOut, rank, size, start, end;
    char msg[9];
    double t0, t1, t2, t3;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Receber parâmetros do servidor de sockets
    receive_params_from_socket(&powmin, &powmax);

    for (int pow = powmin; pow <= powmax; pow++) {
        tam = 1 << pow;

        t0 = wall_time();
        tabulIn  = (int *) malloc ((tam+2)*(tam+2)*sizeof(int));
        tabulOut = (int *) malloc ((tam+2)*(tam+2)*sizeof(int));
        InitTabul(tabulIn, tabulOut, tam);
        t1 = wall_time();

        int lines_per_process = tam / size;
        start = rank * lines_per_process + 1;
        end = (rank + 1) * lines_per_process;

        if (rank == size - 1) {
            end = tam;
        }

        for (i = 0; i < 2 * (tam - 3); i++) {
            UmaVida(tabulIn, tabulOut, tam, start, end);
            MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, tabulOut, (tam+2)*(tam+2)/size, MPI_INT, MPI_COMM_WORLD);
            UmaVida(tabulOut, tabulIn, tam, start, end);
            MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, tabulIn, (tam+2)*(tam+2)/size, MPI_INT, MPI_COMM_WORLD);
        }

        t2 = wall_time();
        if (rank == 0) {
            if (Correto(tabulIn, tam))
                printf("**RESULTADO CORRETO**\n");
            else
                printf("**RESULTADO ERRADO**\n");
            t3 = wall_time();
            printf("tam=%d; tempos: init=%7.7f, comp=%7.7f, fim=%7.7f, tot=%7.7f \n",
                      tam, t1-t0, t2-t1, t3-t2, t3-t0);
        }
        free(tabulIn);
        free(tabulOut);
    }

    MPI_Finalize();
    return 0;
}
