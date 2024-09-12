# Base image
FROM ubuntu:20.04

# Desativar prompts interativos
ENV DEBIAN_FRONTEND=noninteractive

# Atualizar e instalar dependências
RUN apt-get update && \
    apt-get install -y build-essential mpich libopenmpi-dev gcc && \
    apt-get clean && \
    apt-get install -y gcc libcurl4-openssl-dev libopenmpi-dev


# Criar diretório de trabalho para a aplicação
WORKDIR /usr/src/jogo_da_vida

# Copiar os códigos do OpenMP e MPI para o diretório de trabalho do container
COPY ./jogo_da_vida/OMP/jogodavidaomp.c ./OMP/
COPY ./jogo_da_vida/MPI/jogodavidampi.c ./MPI/

# Compilar o código OpenMP
RUN gcc -fopenmp -o ./OMP/jogodavidaomp ./OMP/jogodavidaomp.c -lcurl


# Compilar o código MPI
RUN mpicc -o ./MPI/jogodavidampi ./MPI/jogodavidampi.c -lcurl

# Variável de ambiente para threads OpenMP
ENV OMP_NUM_THREADS=4

# Comando padrão do container
CMD ["bash"]
