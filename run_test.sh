#!/bin/bash

function build_cpp_multi() {
    echo "Building cpp multi"
    cd cpp && mkdir -p bin
    module load gcc-12.2 && g++ -std=c++20 -Wall -O3 -o bin/http_server_multi http_server_multi.cc threadpool.cc
    cd ..
}

# Build if necessary
if [[ ! -x ./cpp/bin/http_server_multi ]]; then
    build_cpp_multi
fi

# Run cpp version and test
cd cpp && echo yo && ./bin/http_server_multi -p 5566 -t 4 &
cpp_pid=$!

# Wait for server to start
sleep 2

# Run test
siege -c 10 -t 20s http://localhost:5566/
echo "This is the results from cpp version."

# Kill server
kill $cpp_pid

# Run Rust version and test
cd rust/server_multi && cargo run -- 0.0.0.0 7878 4 &
rust_pid=$!

# Wait for server to start
sleep 4

# Run test
siege -c 10 -t 20s http://localhost:7878/
echo "This is the results from Rust version."

# Kill server
kill $rust_pid
