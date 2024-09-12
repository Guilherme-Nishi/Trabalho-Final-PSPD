# Trabalho-Final-PSPD

Integrantes:

Carla Rocha Cangussú - 170085023
Gabriel Luiz de Souza Gomes - 190013354
 Guilherme Nishimura da Silva- 200030264
Karla Chaiane da Silva Feliciano - 200021541
Instalar o minikube:

curl -LO https://storage.googleapis.com/minikube/releases/latest/minikube-linux-amd64
sudo install minikube-linux-amd64 /usr/local/bin/minikube

Inicie o cluster com:
minikube start

Buildar a imagem do jogo
docker build -t jogodavida:latest .

aplique a implantação no Kubernetes:

kubectl apply -f elasticsearch-deployment.yaml
kubectl apply -f elasticsearch-service.yaml

kubectl apply -f grafana-deployment.yaml
kubectl apply -f grafana-service.yaml

kubectl apply -f ServiceMonitor.yaml

kubectl apply -f MPI/deployment.yaml
kubectl apply -f MPI/hpa.yaml

kubectl apply -f MPI/deployment.yaml
kubectl apply -f MPI/hpa.yaml

kubectl apply -f OMP/deployment.yaml
kubectl apply -f OMP/hpa.yaml

Obter as urls

minikube service grafana
minikube service elasticsearch

kubectl port-forward svc/grafana 3000:3000 
kubectl port-forward svc/elasticsearch 9200:9200
