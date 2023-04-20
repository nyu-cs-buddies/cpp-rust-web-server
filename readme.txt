# cpp-rust-web-server
A Multicore Project: C++ vs Rust Web Servers

# C++ Version
## How to build on NYU CIMS machines

module load gcc-12.2
cd cpp
g++ -std=c++20 -Wall -pthread -O3 -o bin/http_server_multi http_server_multi.cc threadpool.cc


The exeutable will be available as `cpp/bin/http_server_multi`.

## Usage
The program requires a newer C++ library; therefore, before executing the program, run
module load gcc-12.2

`http_server_multi -h` shows:
Usage: ./bin/http_server_multi [-a ipaddr] [-p port] [-t num_threads (default = 4)] [-d web_root] [-v [verbose]]


Default configs are used if it's run without any arguments. 

Example:
$ ./bin/http_server_multi
>> Listening to 0.0.0.0:8080

# Rust Version
## How to build on NYU CIMS machines
First, we need Rust.
### Install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

After this, restart the terminal or source your `.bashrc`/`.zshrc`.

### Run the program
cd rust/server_multi
cargo run -- <ipaddr> <port> <num_threads> <web_root>


or


cd rust/server_multi
cargo run
# Using default values: 0.0.0.0 8080 4, and serving the default web_root

