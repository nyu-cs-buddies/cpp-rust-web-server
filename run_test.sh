#!/bin/bash

function build_cpp_multi() {
    echo "Building cpp multi"
    cd cpp && mkdir -p bin
    module load gcc-12.2 && g++ -std=c++20 -Wall -pthread -O3 -o bin/http_server_multi http_server_multi.cc threadpool.cc
    cd ..
}

bench_tool_bin=$(command -v ab)

function download_siege() {
  mkdir -p ext_utils && cd ext_utils
  wget http://download.joedog.org/siege/siege-latest.tar.gz
  tar -xf siege-latest.tar.gz
  cd ..
}

function make_siege() {
    cd ext_utils/siege-4.1.6
    ./configure --prefix=$(pwd) && make && make install
    cd ../../
}

# Siege benchmark utility
if command -v siege; then
  bench_tool_bin=$(command -v siege)
elif [[ -x ./ext_utils/siege-4.1.6/bin/siege ]]; then
  bench_tool_bin="$(pwd)/ext_utils/siege-4.1.6/bin/siege"
else
  download_siege
  make_siege
  bench_tool_bin="$(pwd)/ext_utils/siege-4.1.6/bin/siege"
fi

# Build if necessary
if [[ ! -x ./cpp/bin/http_server_multi ]]; then
    build_cpp_multi
fi

# Run cpp version and test
cd cpp && module load gcc-12.2 && ./bin/http_server_multi -p 5566 -t 4 &
cpp_pid=$!

# Wait for server to start
sleep 2

# Run test
${bench_tool_bin} -c 10 -t 20s http://localhost:5566/
echo "This is the results from cpp version."

# Kill server
kill $cpp_pid

# Run Rust version and test
cd rust/server_multi && cargo run -- 0.0.0.0 7878 4 &
rust_pid=$!

# Wait for server to start
sleep 4

# Run test
${bench_tool_bin} -c 10 -t 20s http://localhost:7878/
echo "This is the results from Rust version."

# Kill server
kill $rust_pid
