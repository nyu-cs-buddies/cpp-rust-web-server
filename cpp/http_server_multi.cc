#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <filesystem>
#include <thread>

#include <csignal>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "mime_types.h"
#include "threadpool.h"
// connection
// actions: socket, bind, listen, accept, connect, recv, send, close
// socket, bind, listen
// accept, connect (blocking)
// recv (blocking), send

bool verbose = false;
const int PORT = 8080;
const char IP[] = "0.0.0.0";
const int DEFAULT_NUM_THREADS = 4;

static sig_atomic_t exit_flag = 0;
static int sockfd;

void print_info(const std::string& str) {
  std::cerr << "\033[32;1m[INFO]\033[0m " << str;
}

void print_error(const std::string& str) {
  std::cerr << "\033[31;1m[ERROR]\033[0m " << str;
}

void cleanup_by_break(int sig) {
  std::cout << std::endl;
  print_info("Exiting by break\n");
  exit_flag = 1;
  shutdown(sockfd, SHUT_RDWR);
  signal(SIGINT, SIG_DFL);
}

void handle_connection(int newsockfd) {
  if (verbose) {
    print_info("Connection accepted; newsockfd = " + std::to_string(newsockfd) + "\n");
    std::stringstream tid;
    tid << std::this_thread::get_id();
    print_info("Handled by thread " + tid.str() + "\n");
  }
  char buf[1024] = {0};
  // recv to buf
  int n = recv(newsockfd, buf, 1023, 0);
  if (n < 0) {
      print_error("Error reading from socket!\n");
  }
  if (verbose) {
    std::cout << "=== Received HTTP request ===\n"
              << buf
              << "=============================\n"
              << std::endl;
  }

  auto token = strtok(buf, " \t");
  std::string method{token};
  token = strtok(NULL, " \t");
  std::string uri(token);
  token = strtok(NULL, " \t\r\n");
  std::string protocol(token);

  if (verbose) {
    std::cout << "=== Parsed HTTP request ===" << std::endl
              << "Method = " << method << std::endl
              << "URI = " << uri << std::endl
              << "Protocol = " << protocol << std::endl
              << "===========================" << std::endl;
  }

  // send response with headers and content based on the request
  std::string file_to_serve = "404.html";
  std::string http_status_line = "HTTP/1.1 200 OK\r\n";
  std::string content_type_line = "Content-Type: text/html\r\n";
  if (uri == "/") {
    file_to_serve = "index.html";
  } else if (uri == "/sleep") {
    sleep(5);
    file_to_serve = "index.html";
  } else {
    file_to_serve = uri.substr(1);
  }

  // check if the file exists
  auto filepath = std::filesystem::path(file_to_serve);
  if (!std::filesystem::exists(filepath)) {
    file_to_serve = "404.html";
    http_status_line = "HTTP/1.1 404 Not Found\r\n";
  } else {
    // check the mime type
    auto ext = filepath.extension();
    if (MIME::mime_types.find(ext) != MIME::mime_types.end()) {
      content_type_line = "Content-Type: " + MIME::mime_types.at(ext) + "\r\n";
    } else {
      content_type_line = "Content-Type: application/octet-stream\r\n";
    }
  }
  // ugly way to read files at the moment
  auto file_sz = std::filesystem::file_size(file_to_serve);
  FILE* fp = fopen(file_to_serve.c_str(), "rb");
  unsigned char* send_buf = new unsigned char[file_sz + 1];
  send_buf[file_sz] = 0;
  auto n_fread = fread(send_buf, 1, file_sz, fp);
  if (n_fread != file_sz) {
    std::cerr << "Error on reading file "
      << file_to_serve << "!" << std::endl;
    std::cerr << "file_sz = " << file_sz << "; n_fread = " << n_fread << std::endl;
  }

  // construct the headers
  std::string resp_hdrs =
    http_status_line
    + content_type_line
    + "Content-Length: "
    + std::to_string(file_sz)
    + "\r\n\r\n";

  // send it over the socket
  ssize_t n_send = 0;
  n_send = send(newsockfd, resp_hdrs.c_str(), resp_hdrs.size(), 0);
  if (verbose) print_info("n_send (headers) = " + std::to_string(n_send) + "\n");

  ssize_t rem_file_sz = file_sz;
  auto send_buf_ptr = send_buf;
  while (rem_file_sz > 0) {
    n_send = send(newsockfd, send_buf_ptr, rem_file_sz, 0);
    if (n_send == -1) {
      print_error("Error on send()!\n");
      break;
    }
    send_buf_ptr += n_send;
    rem_file_sz -= n_send;
  }
  if (verbose) print_info("n_send (content) = " + std::to_string(n_send) + "\n");
  delete[] send_buf;
  if (verbose) print_info("Closing connection; newsockfd = " + std::to_string(newsockfd) + "\n");
  close(newsockfd);
  fclose(fp);
  if (verbose) print_info("Connection closed!\n");
}


int main(int argc, char* argv[]) {
    // some default values
    int port = PORT;
    std::string ip = IP;
    int num_threads = DEFAULT_NUM_THREADS;
    auto path = std::filesystem::path("../web_root");

    // args parsing
    int opt;
    while ((opt = getopt(argc, argv, "a:p:t:d:vh")) != -1) {
      switch (opt) {
        case 'a':
          ip = optarg;
          break;
        case 'p':
          port = atoi(optarg);
          break;
        case 't':
          num_threads = atoi(optarg);
          break;
        case 'd':
          path = std::filesystem::path(optarg);
          break;
        case 'v':
          verbose = true;
          break;
        case 'h':
          std::cout << "Usage: "
                    << argv[0]
                    << " [-a ipaddr] [-p port] [-t num_threads] [-d web_root]"
                    << std::endl
                    << "Add -v for verbose mode"
                    << std::endl;
          return 0;
        default:
          std::cerr << "Usage: "
                    << argv[0]
                    << " [-a ipaddr] [-p port] [-t num_threads] [-d web_root]"
                    << std::endl
                    << "Add -v for verbose mode"
                    << std::endl;
          return 1;
      }
    }

    // set the current path for web_root
    std::filesystem::current_path(path);

    print_info("Server starting...\n");

    // signal handler
    signal(SIGINT, cleanup_by_break);

    // create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    } 

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    // server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    server_addr.sin_port = htons(port);

    // bind the socket fd and the address
    if (bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }

    // listen
    listen(sockfd, 128);
    std::cout << "Listening to "
              << inet_ntoa(server_addr.sin_addr) << ":"
              << ntohs(server_addr.sin_port)
              << std::endl;

    // create a thread pool
    ThreadPool pool(num_threads);

    // accept
    while (exit_flag == 0) {
      struct sockaddr_in client_addr;
      socklen_t client_len = sizeof(client_addr);
      int newsockfd = accept(sockfd, (struct sockaddr*) &client_addr, &client_len);
      if (newsockfd < 0) {
          std::cerr << "Error accepting connection (sockfd closed?)" << std::endl;
          break;
      }

      // print the connection
      if (verbose) {
        std::cout << std::endl;
        print_info("New connection! newsockfd = " + std::to_string(newsockfd) + "\n");
        std::cout << "Connection accepted from "
                  << inet_ntoa(client_addr.sin_addr) << ":"
                  << ntohs(client_addr.sin_port) << std::endl;
      }
      // add the task to the thread pool
      pool.add_task(std::bind(handle_connection, newsockfd));
    }

    print_info("Stop accepting connections and finish what's left...\n");
    pool.stop();

    // close the socket
    close(sockfd);
    print_info("Bye!\n");
    return 0;
}
