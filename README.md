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

<!-- Fetch all content in cpp-->
source is current cmake.txt dir, -B build --> build call build

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 

cmake --build build --config Release



<!-- build wihout cache  -->
docker compose build --no-cache
<!-- Rebuild and Starts all container-->
docker compose up --build
<!-- start one time-of indival serivce-->
For link proto
docker compose up proto-sync
<!--start all service  -->
docker compose up
<!-- shut down -->
docker compose down

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