#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <unistd.h>
#include <arpa/inet.h>

#define ind2d(i,j) (i)*(tam+2)+j
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

double wall_time(void) {
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);
  return(tv.tv_sec + tv.tv_usec/1000000.0);
} /* fim-wall_time */

void UmaVida(int* tabulIn, int* tabulOut, int tam) {
  int i, j, vizviv;

  // Paralelize o loop externo com OpenMP
  #pragma omp parallel for private(j, vizviv) shared(tabulIn, tabulOut, tam)
  for (i = 1; i <= tam; i++) {
    for (j = 1; j <= tam; j++) {
      //printf("Número de threads em uso: %d\n", omp_get_num_threads());
      vizviv = tabulIn[ind2d(i-1,j-1)] + tabulIn[ind2d(i-1,j )] +
               tabulIn[ind2d(i-1,j+1)] + tabulIn[ind2d(i ,j-1)] +
               tabulIn[ind2d(i ,j+1)] + tabulIn[ind2d(i+1,j-1)] +
               tabulIn[ind2d(i+1,j )] + tabulIn[ind2d(i+1,j+1)];

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

void DumpTabul(int * tabul, int tam, int first, int last, char* msg){
  int i, ij;
   printf("%s; Dump posições [%d:%d, %d:%d] de tabuleiro %d x %d\n", msg, first, last, first, last, tam, tam);
  for (i=first; i<=last; i++) printf("="); printf("=\n");
  for (i=ind2d(first,0); i<=ind2d(last,0); i+=ind2d(1,0)) {
    for (ij=i+first; ij<=i+last; ij++)
      printf("%c", tabul[ij]? 'X' : '.');
    printf("\n");
  }
  for (i=first; i<=last; i++) printf("="); printf("=\n");
} /* fim-DumpTabul */

void InitTabul(int* tabulIn, int* tabulOut, int tam){
  int ij;
  for (ij=0; ij<(tam+2)*(tam+2); ij++) {
    tabulIn[ij] = 0;
    tabulOut[ij] = 0;
  } /* fim-for */
  tabulIn[ind2d(1,2)] = 1; tabulIn[ind2d(2,3)] = 1;
  tabulIn[ind2d(3,1)] = 1; tabulIn[ind2d(3,2)] = 1;
  tabulIn[ind2d(3,3)] = 1;
} /* fim-InitTabul */

int Correto(int* tabul, int tam){
  int ij, cnt;

  cnt = 0;

  for (ij=0; ij<(tam+2)*(tam+2); ij++)
    cnt = cnt + tabul[ij];
  return (cnt == 5 && tabul[ind2d(tam-2,tam-1)] &&
          tabul[ind2d(tam-1,tam  )] && tabul[ind2d(tam  ,tam-2)] &&
          tabul[ind2d(tam  ,tam-1)] && tabul[ind2d(tam  ,tam  )]);
} /* fim-Correto */

int main(void) {
  omp_set_num_threads(16);
  int powmin, powmax, pow, i, tam, *tabulIn, *tabulOut;
  char msg[9];
  double t0, t1, t2, t3;

  // Receber parâmetros do servidor de sockets
  receive_params_from_socket(&powmin, &powmax);

  // para todos os tamanhos do tabuleiro
  for (pow=powmin; pow<=powmax; pow++) {
    tam = 1 << pow;
    // aloca e inicializa tabuleiros
     t0 = wall_time();
    tabulIn  = (int *) malloc ((tam+2)*(tam+2)*sizeof(int));
    tabulOut = (int *) malloc ((tam+2)*(tam+2)*sizeof(int));
    InitTabul(tabulIn, tabulOut, tam);
    t1 = wall_time();
    for (i=0; i<2*(tam-3); i++) {
      UmaVida(tabulIn, tabulOut, tam);
      UmaVida(tabulOut, tabulIn, tam);
    } /* fim-for */
    t2 = wall_time();

    if (Correto(tabulIn, tam))
      printf("**RESULTADO CORRETO**\n");
    else
      printf("**RESULTADO ERRADO**\n");
    t3 = wall_time();
    printf("tam=%d; tempos: init=%7.7f, comp=%7.7f, fim=%7.7f, tot=%7.7f \n",
           tam, t1-t0, t2-t1, t3-t2, t3-t0);
    free(tabulIn); free(tabulOut);
  }
  return 0;
} /* fim-main */

