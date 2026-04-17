FROM ubuntu:22.04

# Install build tools and compiler
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy the application source code and resources
COPY . .

# Compile the application (Linux build)
RUN g++ -O3 main.cpp -o synapse_server -pthread

# Default port (Render will override this via environment variable)
ENV PORT=8080
EXPOSE 8080

# Production settings
ENV HEADLESS=true

# Run the server
CMD ["./synapse_server"]
