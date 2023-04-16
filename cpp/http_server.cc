#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <thread>
#include <future>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

// Task
// connection
// actions: socket, bind, listen, accept, connect, recv, send, close
// socket, bind, listen
// accept, connect (blocking)
// recv (blocking), send



// Thread pool
// thread picks up a task from the queue
// thread executes the task

// scheduling stuff
// if a client has a higher response time, we can give it a lower priority
// i.e. more connections handled by a thread. Or give this thread lower prio.

// use some threads to accept connections, and the threads put immediately
// the connection into the queue. The queue is then handled by the other
// threads. The handling threads then use async io to recv/send data.
// Can we do like pre-async, async, then the thread put the future into a queue
// and the other threads can fetch it from the queue and wait for it to be ready?

// caching. Multithreading gives shared memory access. We can use this to
// do caching. Think about if we have a DELETE request and GET request.

const int PORT = 8080;
// char HTTP_OK[] = "HTTP/1.1 200 OK\r\n";
// char HTML_CONTENT_TYPE[] = "Content-Type: text/html\r\nContent-Length: 207\r\n\r\n";
const std::string HTTP_OK = "HTTP/1.1 200 OK\r\n";
const std::string HTML_CONTENT_TYPE = "Content-Type: text/html\r\n";

void print_info(const std::string& str) {
  std::cerr << "\033[32;1m[INFO]\033[0m " << str << std::endl;
}

auto accept_conn(int sockfd) {
    std::cout << "Accepting connection via accept_conn in t_accept" << std::endl;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int newsockfd = accept(sockfd, (struct sockaddr*) &client_addr, &client_len);
    if (newsockfd < 0) {
        std::cerr << "Error accepting connection" << std::endl;
        return 1;
    }

    // print the connection
    std::cout << "Connection accepted from "
              << inet_ntoa(client_addr.sin_addr) << ":"
              << ntohs(client_addr.sin_port) << std::endl;

    return newsockfd;
}


int main() {
    std::cout << "Hello world" << std::endl;

    int sockfd, newsockfd, port;
    port = PORT;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    // bind the socket fd and the address
    if (bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }

    // listen
    listen(sockfd, 5);
    std::cout << "Listening on port " << port << std::endl;

    // accept
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    while (true) {
      newsockfd = accept(sockfd, (struct sockaddr*) &client_addr, &client_len);
      if (newsockfd < 0) {
          std::cerr << "Error accepting connection" << std::endl;
          return 1;
      }

      // print the connection
      std::cout << "Connection accepted from "
                << inet_ntoa(client_addr.sin_addr) << ":"
                << ntohs(client_addr.sin_port) << std::endl;

      // std::filesystem::path p{"404.html"};
      char buf[1024] = {0};
      // memset(buf, 0, 1024);
      // recv to buf
      int n = recv(newsockfd, buf, 1023, 0);
      if (n < 0) {
          std::cerr << "Error reading from socket" << std::endl;
          return 1;
      }
      std::cout << "Received:\n" << buf << std::endl;


      // send something
      std::ifstream t("404.html");
      std::stringstream buffer;
      buffer << t.rdbuf();
      // FILE* fp = fopen("404.html", "r");
      // auto fp_op = open("404.html", O_RDONLY);
      ssize_t n_send = 0;
      // n_send = send(newsockfd, HTTP_OK, sizeof(HTTP_OK), 0);
      // print_info("n_send = " + std::to_string(n_send));
      // n_send = send(newsockfd, HTML_CONTENT_TYPE, sizeof(HTML_CONTENT_TYPE), 0);
      // print_info("n_send = " + std::to_string(n_send));
      // if (n_send == -1) print_info("Something's wrong!!");
      // send(newsockfd, "\r\n", 2, 0);
      // std::string content_length = "Content-Length: " + std::to_string(std::filesystem::file_size(p)) + "\r\n\r\n";
      // send(newsockfd, content_length.c_str(), content_length.size(), 0);
      // size_t nread;
      // while ((nread = fread(buf, 1, sizeof(buf), fp)) > 0) {
          // send(newsockfd, buf, nread, 0);
      // }
      std::cout << buffer.str() << std::endl;
      std::string resp = HTTP_OK + HTML_CONTENT_TYPE + "Content-Length: "
        + std::to_string(buffer.str().size()) + "\r\n\r\n" + buffer.str();
      // n_send = send(newsockfd, buffer.str().c_str(), buffer.str().size() + 1, 0);
      n_send = send(newsockfd, resp.c_str(), resp.size() + 1, 0);
      print_info("n_send = " + std::to_string(n_send));
      // sendfile(newsockfd, fp_op, NULL, std::filesystem::file_size(p));
      // send(newsockfd, "\r\n\r\n", 4, 0);
      close(newsockfd);
      // fclose(fp);
    }
    close(sockfd);
    return 0;
}


    // accept by thread
    // std::thread t_accept(&accept_conn, sockfd);
    
    // accept by future
    // auto future = std::async(std::launch::async, accept_conn, sockfd);

    // std::thread t1(send, newsockfd, "Hello curl!\n", 12, 0);
    // std::thread t1(sleep, 10);
    // std::thread t2([]{
    //     std::cout << "Hello from thread 2" << std::endl;
    // });
    // send hello
    // newsockfd = future.get();
    // std::cout << "We're gonna send hello here!" << std::endl;
    // send(newsockfd, "Hello curl!\n", 12, 0);
    // t1.join();
    // t2.join();