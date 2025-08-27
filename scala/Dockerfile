# ===== Stage 1: Cache Dependencies =====
FROM hseeberger/scala-sbt:17.0.2_1.6.2_2.13.8 AS deps

WORKDIR /app

# Copy build files only for caching
COPY build.sbt .
COPY project ./project

# Create dummy src folder to satisfy sbt
RUN mkdir -p src/main/scala

# Download dependencies
RUN sbt update compile

# Create directories for sbt/ivy/cache
RUN mkdir -p /root/.ivy2 /root/.sbt /root/.cache

# ===== Stage 2: Build the fat JAR =====
FROM hseeberger/scala-sbt:17.0.2_1.6.2_2.13.8 AS builder

WORKDIR /app

# Copy build files
COPY build.sbt .
COPY project ./project

# Copy cached dependencies from previous stage
COPY --from=deps /root/.sbt /root/.sbt
COPY --from=deps /root/.ivy2 /root/.ivy2
COPY --from=deps /root/.cache /root/.cache

# Copy source code and build
COPY src ./src
RUN sbt -J-Xmx2g -J-XX:+UseG1GC clean assembly

# ===== Stage 3: Runtime =====
FROM eclipse-temurin:17-jre-alpine

WORKDIR /app

# Install dumb-init for proper PID handling
RUN apk add --no-cache dumb-init

# Create non-root user
RUN addgroup -g 1001 -S appgroup && \
    adduser -u 1001 -S appuser -G appgroup

# Copy fat JAR from builder
COPY --from=builder /app/target/scala-2.13/scala-assembly-0.1.0-SNAPSHOT.jar app.jar

# Set ownership
RUN chown -R appuser:appgroup /app
USER appuser

# Optimized runtime

ENTRYPOINT ["java", \
    "-XX:+UseContainerSupport", \
    "-XX:MaxRAMPercentage=75.0", \
    "-XX:+UseG1GC", \
    "-XX:+UseStringDeduplication", \
    "-XX:+UnlockExperimentalVMOptions", \
    "-Djava.security.egd=file:/dev/./urandom", \
    "-jar", \
    "app.jar"]
