# Base image
FROM ubuntu:20.04

# Desativar prompts interativos
ENV DEBIAN_FRONTEND=noninteractive

# Atualizar e instalar dependências
RUN apt-get update && apt-get install -y build-essential gcc libpthread-stubs0-dev

# Criar diretório de trabalho para a aplicação
WORKDIR /usr/src/socket

# Copiar o código do servidor socket
COPY ./socket_server.c .

# Compilar o servidor socket
RUN gcc -o socket_server socket_server.c -lpthread

# Comando padrão do container
CMD ["./socket_server"]
