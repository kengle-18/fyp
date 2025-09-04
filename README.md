# fyp

in python
docker build -t python-app .
docker run --rm python-app  

docker run --rm python-app <py name>

in scala
sbt clean compile assembly

docker build -t scala-app . 
docker run --rm scala-app
docker run -it --rm scala-app
docker run -it scala-app

in cpp
requirer to cmake first then check Fetch all content in cpp

docker build -t cpp-app .
docker run --rm cpp-app
docker run cpp-app

<!-- Check platfrom -->
docker inspect --format '{{.Os}}/{{.Architecture}}' your_image_names

docker inspect --format '{{.Os}}/{{.Architecture}}' scala-app       --> linux/amd64

<!-- Fetch all content in cpp obsolete use docker for all works-->
source is current cmake.txt dir, -B build --> build call build

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 
cmake --build build --config Release



<!-- build wihout cache  -->
docker compose build --no-cache
<!-- Rebuild and Starts all container-->
docker compose up --build
<!-- Restart and build one container -->
docker compose up --build scala-app
<!-- start one time-of indival serivce-->
For link proto
docker compose up proto-sync
docker compose up cpp-app
<!--start all service w/o new imange -->
docker compose up
<!-- shut down -->
docker compose down
<!-- Docker ps -->
docker ps

<!-- Run in detach -->
docker-compose up -d cpp-app
docker start -ai cpp-app
<!-- Enter shell in cpp -->
docker run -it --entrypoint /bin/bash cpp-app
Check volume
docker run -it --entrypoint /bin/bash -v C:/Users/user/Downloads/fyp/cpp/src:/app/src ubuntu  

<!-- Debug docker cpp-->
docker build -t cpp-app:builder --target builder .
docker run -it --rm cpp-app:builder /bin/bash
<!-- Runtime cpp -->
docker run -it --rm --entrypoint /bin/bash cpp-app:latest

<!-- Run in docker container  -->
when the conatiner is running
docker run -it --name cpp-app-debug cpp-app bash
<!-- Check version of deps -->
docker run --rm ubuntu:22.04 bash -c "\
  apt-get update && \
  apt-get install -y libgrpc++-dev libprotobuf-dev protobuf-compiler protobuf-compiler-grpc && \
  echo 'Protobuf version:' && protoc --version && \
  echo 'libgrpc++-dev version:' && dpkg -s libgrpc++-dev | grep Version && \
  echo 'libprotobuf-dev version:' && dpkg -s libprotobuf-dev | grep Version"

Protobuf version:
libprotoc 3.12.4
libgrpc++-dev version:
Version: 1.30.2-3build6
libprotobuf-dev version:
Version: 3.12.4-1ubuntu7.22.04.4


docker-compose.ynml
:ro --> read only 
voulumes: affect local too

<!-- format -->
sbt scalafmtAll

<!-- check EOF LF or CRLF -->
git ls-files --eol


<!-- Activate conda before python -->
activate base

What’s META-INF/services?

In Java (and also in Scala), ServiceLoader is a way to discover implementations of an interface at runtime without hardcoding them.

Each file in META-INF/services is named after an interface or abstract class and contains the fully qualified class names of implementations.

gRPC uses this internally to discover available channel providers, transport factories, etc.

Example: