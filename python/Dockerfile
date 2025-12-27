# Use an official Python runtime as a base image
FROM python:3.11-slim

# Set working directory
WORKDIR /app

# Install watchdog dependency
RUN pip install --no-cache-dir watchdog

# Copy your Python script into the container
COPY src/ .

# Run your script automatically when the container starts
ENTRYPOINT ["python"]
 