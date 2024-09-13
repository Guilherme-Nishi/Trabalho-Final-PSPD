from pyspark import SparkContext, SparkConf
import numpy as np
import time
import logging

# Configuração do nível de log para o Spark
conf = SparkConf().setAppName("GameOfLife").setMaster("local[2]")

POWMIN = 2
POWMAX = 6

def ind2d(i, j, tam):
    return i * (tam + 2) + j

def wall_time():
    return time.time()

def UmaVidaParallel(tabulIn, tam):
    def process_cell(idx):
        i, j = idx
        vizviv = (tabulIn[ind2d(i - 1, j - 1, tam)] + tabulIn[ind2d(i - 1, j, tam)] +
                  tabulIn[ind2d(i - 1, j + 1, tam)] + tabulIn[ind2d(i, j - 1, tam)] +
                  tabulIn[ind2d(i, j + 1, tam)] + tabulIn[ind2d(i + 1, j - 1, tam)] +
                  tabulIn[ind2d(i + 1, j, tam)] + tabulIn[ind2d(i + 1, j + 1, tam)])
        if tabulIn[ind2d(i, j, tam)] and vizviv < 2:
            return (i, j, 0)
        elif tabulIn[ind2d(i, j, tam)] and vizviv > 3:
            return (i, j, 0)
        elif not tabulIn[ind2d(i, j, tam)] and vizviv == 3:
            return (i, j, 1)
        else:
            return (i, j, tabulIn[ind2d(i, j, tam)])

    indices = [(i, j) for i in range(1, tam + 1) for j in range(1, tam + 1)]
    rdd = sc.parallelize(indices)
    result = rdd.map(process_cell).collect()

    tabulOut = np.zeros((tam + 2) * (tam + 2), dtype=int)
    for i, j, value in result:
        tabulOut[ind2d(i, j, tam)] = value

    return tabulOut

def InitTabul(tabulIn, tabulOut, tam):
    for i in range((tam + 2) * (tam + 2)):
        tabulIn[i] = 0
        tabulOut[i] = 0
    tabulIn[ind2d(1, 2, tam)] = 1
    tabulIn[ind2d(2, 3, tam)] = 1
    tabulIn[ind2d(3, 1, tam)] = 1
    tabulIn[ind2d(3, 2, tam)] = 1
    tabulIn[ind2d(3, 3, tam)] = 1

def Correto(tabul, tam):
    cnt = sum(tabul)
    return (cnt == 5 and tabul[ind2d(tam - 2, tam - 1, tam)] and
            tabul[ind2d(tam - 1, tam, tam)] and tabul[ind2d(tam, tam - 2, tam)] and
            tabul[ind2d(tam, tam - 1, tam)] and tabul[ind2d(tam, tam, tam)])

if __name__ == "__main__":
    conf = SparkConf().setAppName("GameOfLife").setMaster("local[2]")
    sc = SparkContext(conf=conf)
    sc.setLogLevel("ERROR")

    for pow in range(POWMIN, POWMAX + 1):
        tam = 1 << pow

        t0 = wall_time()
        tabulIn = np.zeros((tam + 2) * (tam + 2), dtype=int)
        tabulOut = np.zeros((tam + 2) * (tam + 2), dtype=int)
        InitTabul(tabulIn, tabulOut, tam)
        t1 = wall_time()

        stable = False
        iteration = 0
        while iteration < 2 * (tam - 3) and not stable:
            tabulOut = UmaVidaParallel(tabulIn, tam)
            new_tabulIn = UmaVidaParallel(tabulOut, tam)

            if np.array_equal(tabulIn, new_tabulIn):
                stable = True

            tabulIn = new_tabulIn
            iteration += 1

        t2 = wall_time()

        if Correto(tabulIn, tam):
            print("**Ok, RESULTADO CORRETO**")
        else:
            print("**Nok, RESULTADO ERRADO**")

        t3 = wall_time()
        print(f"tam={tam}; tempos: init={t1 - t0:.7f}, comp={t2 - t1:.7f}, fim={t3 - t2:.7f}, tot={t3 - t0:.7f}")

    sc.stop()
