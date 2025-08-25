# Stage 1: The Build Stage
FROM hseeberger/scala-sbt:17.0.2_1.6.2_2.13.8 AS builder

# Set the working directory inside the container
WORKDIR /app

# Copy build definition files first to leverage build cache
COPY build.sbt .
COPY project ./project

# Download dependencies
# The `update` task downloads all dependencies, which can then be cached.
RUN sbt update

# Copy source code and perform the final build
COPY src ./src
RUN sbt assembly

# Stage 2: The Runtime Stage
FROM eclipse-temurin:17-jre-focal

# Set the working directory
WORKDIR /app

# Copy the fat JAR from the builder stage
COPY --from=builder /app/target/scala-2.13/scala-assembly-0.1.0-SNAPSHOT.jar .

# The command to run the application
CMD ["java", "-jar", "scala-assembly-0.1.0-SNAPSHOT.jar"]