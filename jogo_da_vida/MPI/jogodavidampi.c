#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <curl/curl.h>

#define ind2d(i,j) ((i)*(tam+2)+(j))
#define POWMIN 2
#define POWMAX 12

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

    curl = curl_easy_init();
    if(curl) {
        char json_data[256];
        snprintf(json_data, sizeof(json_data),
                 "{\"tam\": %d, \"init_time\": %.7f, \"comp_time\": %.7f, \"total_time\": %.7f}",
                 tam, init_time, comp_time, total_time);

        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9200/game-of-life/_doc/");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "Erro ao enviar dados para o Elasticsearch: %s\n", curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    }
}

int main(int argc, char** argv) {
    int pow, i, tam, *tabulIn, *tabulOut, rank, size;
    double t0, t1, t2, t3;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    for (pow = POWMIN; pow <= POWMAX; pow++) {
        tam = 1 << pow;

        t0 = wall_time();
        tabulIn  = (int ) malloc((tam+2)(tam+2)*sizeof(int));
        tabulOut = (int ) malloc((tam+2)(tam+2)*sizeof(int));

        t1 = wall_time();
        for (i = 0; i < 2 * (tam - 3); i++) {
            // Computações de vida
        }
        t2 = wall_time();

        if (rank == 0) {
            t3 = wall_time();
            send_to_elasticsearch(tam, t1-t0, t2-t1, t3-t0);
        }

        free(tabulIn);
        free(tabulOut);
    }

    MPI_Finalize();
    return 0;
}