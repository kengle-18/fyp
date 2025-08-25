# Stage 1: The Build Stage
FROM hseeberger/scala-sbt:17.0.2_1.6.2_2.13.8 AS builder

# Set the working directory inside the container
WORKDIR /app

# Copy the build files and source code
COPY build.sbt .
COPY project ./project
COPY src ./src

# Use sbt to compile the code and create a fat JAR (an assembly)
RUN sbt clean assembly

# Stage 2: The Runtime Stage
FROM eclipse-temurin:17-jre-focal

# Set the working directory
WORKDIR /app

# Copy the fat JAR from the builder stage
COPY --from=builder /app/target/scala-2.13/scala-assembly-0.1.0-SNAPSHOT.jar .

# The command to run the application
CMD ["java", "-jar", "scala-assembly-0.1.0-SNAPSHOT.jar"]