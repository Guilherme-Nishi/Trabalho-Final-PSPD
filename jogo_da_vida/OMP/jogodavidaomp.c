#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <string.h>
#include <curl/curl.h> // Inclua a biblioteca curl para HTTP requests

#define ind2d(i,j) ((i)*(tam+2)+(j))

double wall_time(void) {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return(tv.tv_sec + tv.tv_usec/1000000.0);
} /* fim-wall_time */

// Função para enviar dados para o Elasticsearch
void SendDataToElasticsearch(const char* data) {
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9200/indice/_doc/");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    curl_global_cleanup();
}

void UmaVida(int* tabulIn, int* tabulOut, int tam) {
    int i, j, vizviv;
    // Paralelize o loop externo com OpenMP
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
} /* fim-DumpTabul */

void InitTabul(int* tabulIn, int* tabulOut, int tam) {
    int ij;
    for (ij = 0; ij < (tam+2)*(tam+2); ij++) {
        tabulIn[ij] = 0;
        tabulOut[ij] = 0;
    }
    tabulIn[ind2d(1,2)] = 1; tabulIn[ind2d(2,3)] = 1;
    tabulIn[ind2d(3,1)] = 1; tabulIn[ind2d(3,2)] = 1;
    tabulIn[ind2d(3,3)] = 1;
} /* fim-InitTabul */

int Correto(int* tabul, int tam) {
    int ij, cnt;
    cnt = 0;
    for (ij = 0; ij < (tam+2)*(tam+2); ij++)
        cnt = cnt + tabul[ij];
    return (cnt == 5 && tabul[ind2d(tam-2,tam-1)] &&
            tabul[ind2d(tam-1,tam  )] && tabul[ind2d(tam  ,tam-2)] &&
            tabul[ind2d(tam  ,tam-1)] && tabul[ind2d(tam  ,tam  )]);
} /* fim-Correto */

int main(int argc, char** argv) {
    omp_set_num_threads(16);
    int pow, i, tam, *tabulIn, *tabulOut;
    char msg[9];
    double t0, t1, t2, t3;
    int powmin = atoi(argv[1]);
    int powmax = atoi(argv[2]);

    // para todos os tamanhos do tabuleiro
    for (pow = powmin; pow <= powmax; pow++) {
        tam = 1 << pow;
        // aloca e inicializa tabuleiros
        t0 = wall_time();
        tabulIn  = (int *) malloc ((tam+2)*(tam+2)*sizeof(int));
        tabulOut = (int *) malloc ((tam+2)*(tam+2)*sizeof(int));
        InitTabul(tabulIn, tabulOut, tam);
        t1 = wall_time();
        for (i = 0; i < 10; i++) {
            UmaVida(tabulIn, tabulOut, tam);
            UmaVida(tabulOut, tabulIn, tam);
        }
        t2 = wall_time();

        if (Correto(tabulIn, tam))
            printf("**OpenMP - RESULTADO CORRETO**\n");
        else
            printf("**OpenMP - RESULTADO ERRADO**\n");
        t3 = wall_time();
        printf("tam=%d; tempos: init=%7.7f, comp=%7.7f, fim=%7.7f, tot=%7.7f \n",
               tam, t1-t0, t2-t1, t3-t2, t3-t0);

        // Enviar dados para o Elasticsearch
        char postdata[1024];
        snprintf(postdata, sizeof(postdata), "{\"index\": \"jogo_da_vida\", \"data\": [");
        for (i = 0; i < (tam+2)*(tam+2); i++) {
            snprintf(postdata + strlen(postdata), sizeof(postdata) - strlen(postdata),
                     "{\"position\": %d, \"value\": %d},", i, tabulIn[i]);
        }
        postdata[strlen(postdata) - 1] = ']'; // Substituir a última vírgula por colchete
        strcat(postdata, "}");

        // Adicione a função de envio de dados
        SendDataToElasticsearch(postdata);

        free(tabulIn);
        free(tabulOut);
    }
    return 0;
} /* fim-main */
