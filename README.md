# fyp

in python
docker build -t python-app .
docker run --rm python-app  

in scala
sbt clean compile assembly

docker build -t scala-app . 
docker run --rm scala-app
docker run -it --rm scala-app
docker run -it scala-app

in cpp
docker build -t cpp-app .
docker run --rm cpp-app

<!-- build wihout cache  -->
docker compose build --no-cache
<!-- Starts container -->
remains running 
docker compose up --build
<!-- shut down -->
docker compose down

<!-- format -->
sbt scalafmtAll

<!-- check EOF LF or CRLF -->
git ls-files --eol


What’s META-INF/services?

In Java (and also in Scala), ServiceLoader is a way to discover implementations of an interface at runtime without hardcoding them.

Each file in META-INF/services is named after an interface or abstract class and contains the fully qualified class names of implementations.

gRPC uses this internally to discover available channel providers, transport factories, etc.

Example: