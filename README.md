# cpp-rust-web-server
A Multicore Project: C++ vs Rust Web Servers

# C++ Version
## How to build on NYU CIMS machines
```bash
module load gcc-12.2
cd cpp
g++ -std=c++20 -Wall -pthread -O3 -o bin/http_server_multi http_server_multi.cc threadpool.cc
```

The exeutable will be available as `cpp/bin/http_server_multi`.

## Usage
The program requires a newer C++ library; therefore, before executing the program, run
```
module load gcc-12.2
```

`http_server_multi -h` shows:
```
Usage: ./bin/http_server_multi [-a ipaddr] [-p port] [-t num_threads (default = 4)] [-d web_root] [-v [verbose]]
```

Default configs are used if it's run without any arguments. 

Example:
```bash
$ ./bin/http_server_multi
Listening to 0.0.0.0:8080
```

# Rust Version
## How to build on NYU CIMS machines
First, we need Rust.
### Install Rust
```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```
After this, restart the terminal or source your `.bashrc`/`.zshrc`.

### Run the program
```
cd rust/server_multi
cargo run -- <ipaddr> <port> <num_threads> <web_root>
```

or

```
cd rust/server_multi
cargo run
# Using default values: 0.0.0.0 8080 4, and serving the default web_root
```


# Performance test script
```
./run_test.sh
```

The script will run Type-A, Type-B and Type-C with different problem sizes and number of threads specified in our reports for both C++ and Rust versions. We use `siege` to stress the programs and also gather results from its output.

The output is in the following format. This was stressing the C++ version server for 20 seconds with 4 threads and 1 MB webpage size.
```
Transactions:		        15076 hits
Availability:		        100.00 %
Elapsed time:		        20.97 secs
Data transferred:	        14377.59 MB
Response time:		        0.01 secs
Transaction rate:	        718.93 trans/sec
Throughput:		            685.63 MB/sec
Concurrency:		        3.86
Successful transactions:    15076
Failed transactions:	    0
Longest transaction:	    0.07
Shortest transaction:	    0.00
```
