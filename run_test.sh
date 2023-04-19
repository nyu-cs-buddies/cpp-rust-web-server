#!/bin/bash

# fallback to ab on the system
bench_tool_bin=$(command -v ab)

# configs
base_url="http://localhost"
port_cpp=$((${RANDOM} + 5566))
port_rust=$((${RANDOM} + 7878))
fixed_num_threads=4
num_threads_list=(1 2 4 8 16 32 64 128)
siege_concurrencies=(1 2 4 8 16 32 64 128)
siege_time="60s"
siege_url_file="siege_test_urls.txt"
web_root="web_root"
files_dir="siege_tests_files"
file_sizes=(100  1000 10000 100000 1000000 10000000)
#           100B 1KB  10KB  100KB  1MB     10MB
total_requests_size=(1000 5000 100000 25000 50000)
test_results_dir="test_results"
mkdir -p ${test_results_dir}

function generate_files() {
    echo "Generating files filled with random data"
    mkdir -p ${files_dir}
    for size in ${file_sizes[@]}; do
        head -c ${size} < /dev/urandom > ${web_root}/${files_dir}/${size}
    done
    echo "Done generating test files!"
}

function generate_urls() {
    # to be implemented
}


function build_cpp_multi() {
    echo "Building cpp multi"
    cd cpp && mkdir -p bin
    module load gcc-12.2 && g++ -std=c++20 -Wall -pthread -O3 -o bin/http_server_multi http_server_multi.cc threadpool.cc
    cd ..
}

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

generate_files

#### Cpp ####
# Run test: Type A
for req_size in ${total_requests_size[@]}; do
    for num_threads in ${num_threads_list[@]}; do
        port_cpp=$((${RANDOM} + 5566))
        # Run cpp version and test
        cd cpp && module load gcc-12.2 && ./bin/http_server_multi -p ${port_cpp} -t num_threads &
        cpp_pid=$!
        cd ..

        # Wait for server to start
        sleep 2

        # Start the siege
        ${bench_tool_bin} -c ${num_threads} \
            -r $((req_size / num_threads)) \
            ${base_url}:${port_cpp}/${files_dir}/${files[2]} \
            2>&1 | tee tmp.txt
        
        result_file="${test_results_dir}/cpp_n_threads_${num_threads}_reqsize_${req_size}_filesize_${files[2]}.txt"
        echo "This is the results from cpp version with ${num_threads} threads, ${req_size} reqs and ${files[2]} file size." > ${result_file}
        tail -n 20 tmp.txt >> ${result_file}
        rm tmp.txt
        kill -9 $cpp_pid
    done
done

# Run test: Type B 
for req_size in ${total_requests_size[@]}; do
    for file_size in ${file_sizes[@]}; do
        port_cpp=$((${RANDOM} + 5566))
        # Run cpp version and test
        cd cpp && module load gcc-12.2 && ./bin/http_server_multi -p ${port_cpp} -t ${fixed_num_threads} &
        cpp_pid=$!
        cd ..

        # Wait for server to start
        sleep 2

        # Start the siege
        ${bench_tool_bin} -c ${fixed_num_threads} \
            -r $((req_size / num_threads)) \
            ${base_url}:${port_cpp}/${files_dir}/${file_size} \
            2>&1 | tee tmp.txt
        
        result_file="${test_results_dir}/cpp_n_threads_${fixed_num_threads}_reqsize_${req_size}_filesize_${file_size}.txt"
        echo "This is the results from cpp version with ${fixed_num_threads} threads, ${req_size} reqs and ${file_size} file size." > ${result_file}
        tail -n 20 tmp.txt >> ${result_file}
        rm tmp.txt
        kill -9 $cpp_pid
    done
done

# Run test: Type C
for file_size in ${file_sizes[@]}; do
    for num_threads in ${num_threads_list[@]}; do
        port_cpp=$((${RANDOM} + 5566))
        # Run cpp version and test
        cd cpp && module load gcc-12.2 && ./bin/http_server_multi -p ${port_cpp} -t num_threads &
        cpp_pid=$!
        cd ..

        # Wait for server to start
        sleep 2

        # Start the siege
        ${bench_tool_bin} -c ${num_threads} -t ${siege_time} \
            ${base_url}:${port_cpp}/${files_dir}/${file_size} \
            2>&1 | tee tmp.txt
        
        result_file="${test_results_dir}/cpp_n_threads_${num_threads}_filesize_${file_size}_siege_time_${siege_time}.txt"
        echo "This is the results from cpp version with ${num_threads} threads, ${file_size} file size and ${siege_time} siege time." > ${result_file}
        tail -n 20 tmp.txt >> ${result_file}
        rm tmp.txt
        kill -9 $cpp_pid
    done
done

# Run test: Type D
# to be done


#### Rust ####
# Run test: Type A
for req_size in ${total_requests_size[@]}; do
    for num_threads in ${num_threads_list[@]}; do
        port_rust=$((${RANDOM} + 7878))
        # Run rust version and test
        cd rust/server_multi && cargo run -- 0.0.0.0 ${port_rust} ${num_threads} &
        rust_pid=$!
        cd ..

        # Wait for server to start
        sleep 6

        # Start the siege
        ${bench_tool_bin} -c ${num_threads} \
            -r $((req_size / num_threads)) \
            ${base_url}:${port_rust}/${files_dir}/${files[2]} \
            2>&1 | tee tmp.txt
        
        result_file="${test_results_dir}/rust_n_threads_${num_threads}_reqsize_${req_size}_filesize_${files[2]}.txt"
        echo "This is the results from rust version with ${num_threads} threads, ${req_size} reqs and ${files[2]} file size." > ${result_file}
        tail -n 20 tmp.txt >> ${result_file}
        rm tmp.txt
        kill -9 $rust_pid
    done
done

# Run test: Type B 
for req_size in ${total_requests_size[@]}; do
    for file_size in ${file_sizes[@]}; do
        port_rust=$((${RANDOM} + 7878))
        # Run rust version and test
        cd rust/server_multi && cargo run -- 0.0.0.0 ${port_rust} ${num_threads} &
        rust_pid=$!
        cd ..

        # Wait for server to start
        sleep 6

        # Start the siege
        ${bench_tool_bin} -c ${fixed_num_threads} \
            -r $((req_size / num_threads)) \
            ${base_url}:${port_rust}/${files_dir}/${file_size} \
            2>&1 | tee tmp.txt
        
        result_file="${test_results_dir}/rust_n_threads_${fixed_num_threads}_reqsize_${req_size}_filesize_${file_size}.txt"
        echo "This is the results from rust version with ${fixed_num_threads} threads, ${req_size} reqs and ${file_size} file size." > ${result_file}
        tail -n 20 tmp.txt >> ${result_file}
        rm tmp.txt
        kill -9 $rust_pid
    done
done

# Run test: Type C
for file_size in ${file_sizes[@]}; do
    for num_threads in ${num_threads_list[@]}; do
        pport_rust=$((${RANDOM} + 7878))
        # Run rust version and test
        cd rust/server_multi && cargo run -- 0.0.0.0 ${port_rust} ${num_threads} &
        rust_pid=$!
        cd ..

        # Wait for server to start
        sleep 6

        # Start the siege
        ${bench_tool_bin} -c ${num_threads} -t ${siege_time} \
            ${base_url}:${port_rust}/${files_dir}/${file_size} \
            2>&1 | tee tmp.txt
        
        result_file="${test_results_dir}/rust_n_threads_${num_threads}_filesize_${file_size}_siege_time_${siege_time}.txt"
        echo "This is the results from rust version with ${num_threads} threads, ${file_size} file size and ${siege_time} siege time." > ${result_file}
        tail -n 20 tmp.txt >> ${result_file}
        rm tmp.txt
        kill -9 $rust_pid
    done
done
