use std::thread;
use std::time::Duration;
use std::fs;
use std::net::TcpListener;
use std::net::TcpStream;
use std::io::prelude::*;
use std::env;


fn main() {
    // some default values here
    let mut addr = "0.0.0.0";
    let mut port = "8080";
    let mut num_threads: usize = 4;
    // parsing the command line arguments
    let args: Vec<String> = env::args().collect();
    if args.len() != 4 {
        println!("Usage: {} <addr> <port> <num_threads>", args[0]);
        println!("Using default values: {} {} {}", addr, port, num_threads);
    } else {
        addr = &args[1];
        port = &args[2];
        num_threads = args[3].parse::<usize>().unwrap();
    }

    let listen_to = format!("{}:{}", addr, port);

    // listening to the TCP connection
    let listener = 
    TcpListener::bind(listen_to).unwrap();

    let pool = server_multi::ThreadPool::new(num_threads);

    for stream in listener.incoming() {
        let stream = stream.unwrap();
        pool.execute(|| {
            handle_connection(stream);
        });
    }

    println!("Shutting down.");
}

fn handle_connection(mut stream: TcpStream) {
    // create a buffer to hold the data
    let mut buffer = [0; 1024];
    stream.read( &mut buffer).unwrap();

    let get = b"GET / HTTP/1.1\r\n";
    let sleep = b"GET /sleep HTTP/1.1\r\n";
    let pic_404 = b"GET /imsorry.jpg HTTP/1.1\r\n";

    let(status_line, filename, mime_type) = 
        if buffer.starts_with(get) {
            ("HTTP/1.1 200 OK", "index.html", "text/html")
        } 
        else if buffer.starts_with(sleep) {
            thread::sleep(Duration::from_secs(5));
            ("HTTP/1.1 200 OK", "index.html", "text/html")
        }
        else if buffer.starts_with(pic_404) {
            ("HTTP/1.1 200 OK", "imsorry.jpg", "image/jpg")
        }
        else {
            ("HTTP/1.1 404 NOT FOUND", "404.html", "text/html")
        };

    // let contents = fs::read_to_string(filename).unwrap();
    let contents = fs::read(filename).unwrap();

    let response = format!(
        "{}\r\nContent-Type: {}\r\nContent-Length: {}\r\n\r\n",
        status_line,
        mime_type,
        contents.len()
    );

    stream.write(response.as_bytes()).unwrap();
    stream.write(&contents).unwrap();
    stream.flush().unwrap();
}
