FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    software-properties-common \
    wget \
    unzip \
    build-essential \
    libomp-dev \
    python3 \
    python3-pip \
    openjdk-11-jdk \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*


# Descomentar abaixo se não estiver usando mac com chips m
ENV JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64

# Descomentar abaixo se estivar usando mac com chips m
# ENV JAVA_HOME=/usr/lib/jvm/java-11-openjdk-arm64
ENV PATH=$PATH:$JAVA_HOME/bin

RUN pip3 install pyspark --no-cache-dir --break-system-packages
RUN pip3 install elasticsearch --no-cache-dir --break-system-packages
RUN pip3 install numpy --no-cache-dir --break-system-packages

# Set working directory
WORKDIR /opt/spark-apps

# Copy the application file into the container
COPY jogospark.py .

# Set the default command to run the application
CMD ["spark-submit","jogospark.py"]