#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <curl/curl.h>

#define ind2d(i,j) (i)*(tam+2)+j
#define POWMIN 2
#define POWMAX 10

double wall_time(void) {
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  return(tv.tv_sec + tv.tv_usec/1000000.0);
}

// Função para enviar dados ao Elasticsearch
void send_to_elasticsearch(int tam, double init_time, double comp_time, double total_time) {
  CURL *curl;
  CURLcode res;
  
  // Inicializa o cURL
  curl = curl_easy_init();
  if(curl) {
    // Monta o JSON com os dados
    char json_data[256];
    snprintf(json_data, sizeof(json_data),
             "{\"tam\": %d, \"init_time\": %.7f, \"comp_time\": %.7f, \"total_time\": %.7f}",
             tam, init_time, comp_time, total_time);
    
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9200/game-of-life/_doc/");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    
    // Executa a requisição
    res = curl_easy_perform(curl);
    
    // Verifica o resultado
    if(res != CURLE_OK)
      fprintf(stderr, "Erro ao enviar dados para o Elasticsearch: %s\n", curl_easy_strerror(res));
    
    // Finaliza cURL
    curl_easy_cleanup(curl);
  }
}

void UmaVida(int* tabulIn, int* tabulOut, int tam) {
  int i, j, vizviv;
  #pragma omp parallel for private(j, vizviv) shared(tabulIn, tabulOut, tam)
  for (i = 1; i <= tam; i++) {
    for (j = 1; j <= tam; j++) {
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

int main(void) {
  omp_set_num_threads(16);
  int pow, i, tam, *tabulIn, *tabulOut;
  double t0, t1, t2, t3;

  for (pow = POWMIN; pow <= POWMAX; pow++) {
    tam = 1 << pow;

    t0 = wall_time();
    tabulIn  = (int ) malloc((tam+2)(tam+2)*sizeof(int));
    tabulOut = (int ) malloc((tam+2)(tam+2)*sizeof(int));

    t1 = wall_time();
    for (i = 0; i < 2*(tam-3); i++) {
      UmaVida(tabulIn, tabulOut, tam);
      UmaVida(tabulOut, tabulIn, tam);
    }
    t2 = wall_time();

    t3 = wall_time();

    printf("tam=%d; tempos: init=%7.7f, comp=%7.7f, fim=%7.7f, tot=%7.7f \n",
           tam, t1-t0, t2-t1, t3-t2, t3-t0);

    // Enviar dados para Elasticsearch
    send_to_elasticsearch(tam, t1-t0, t2-t1, t3-t0);

    free(tabulIn);
    free(tabulOut);
  }

  return 0;
}