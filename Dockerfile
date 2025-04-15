FROM alpine:3.18 as build

# Install build dependencies
RUN apk add --no-cache \
    build-base \
    clang \
    cmake \
    git \
    curl-dev \
    openssl-dev \
    zlib-dev \
    boost-dev \
    bsd-compat-headers
    

# Set the working directory
WORKDIR /code
# Copy the source code
COPY ./src /code/
# Build the application using clang
RUN CC=clang CXX=clang++ cmake .
RUN CC=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF .
RUN cmake --build .

# Create a slimmer runtime container
FROM alpine:3.18
# Install runtime dependencies
RUN apk add --no-cache libcurl openssl zlib boost
# Set the working directory
WORKDIR /app
# Copy the built application from the build container
COPY --from=build /code/dlbot .

ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Set the entry point
ENTRYPOINT ["./dlbot"]