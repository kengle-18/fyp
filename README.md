# fyp

in python
docker build -t python-app .
docker run --rm python-app  

in scala
docker build -t scala-app . 
docker run --rm scala-app

in cpp
docker build -t cpp-app .
docker run --rm cpp-app

<!-- build wihout cache  -->
docker compose build --no-cache
<!-- Starts container -->
docker compose up --build
<!-- shut down -->
docker compose down