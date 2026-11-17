# -------------------------------------------------------------
# Multi-stage Dockerfile for Personal Expense Analyzer
# -------------------------------------------------------------

# Stage 1: Build stage
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ \
    make \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy source code
COPY . .

# Compile the C++ backend
RUN make clean && make build

# -------------------------------------------------------------
# Stage 2: Lightweight runtime stage
FROM ubuntu:22.04 AS runner

WORKDIR /app

# Copy binary from builder
COPY --from=builder /build/expense_server .

# Copy frontend static files
COPY --from=builder /build/frontend ./frontend

# Copy default initial storage structure
COPY --from=builder /build/storage ./storage

# Persistent volume for application data files
VOLUME ["/app/storage"]

# HTTP Server Port
EXPOSE 8080

# Run the server
CMD ["./expense_server"]
