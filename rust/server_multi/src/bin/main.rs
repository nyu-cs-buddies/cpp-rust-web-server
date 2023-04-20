use std::thread;
use std::time::Duration;
use std::fs;
use std::net::TcpListener;
use std::net::TcpStream;
use std::io::prelude::*;
use std::env;
use std::path::Path;

fn main() {
    // some default values here
    let mut addr = "0.0.0.0";
    let mut port = "8080";
    let mut num_threads: usize = 4;
    // parsing the command line arguments
    let args: Vec<String> = env::args().collect();
    if args.len() == 5 {
        addr = &args[1];
        port = &args[2];
        num_threads = args[3].parse::<usize>().unwrap();
        let web_dir = Path::new(&args[4]);
        assert!(env::set_current_dir(&web_dir).is_ok());
    } else if args.len() == 4 {
        addr = &args[1];
        port = &args[2];
        num_threads = args[3].parse::<usize>().unwrap();
        let web_dir = Path::new("../../web_root");
        assert!(env::set_current_dir(&web_dir).is_ok());
    } else {
        println!("Usage: cargo run -- <ipaddr> <port> <num_threads> <web_root>");
        println!("Using default values: {} {} {}, and serving the default web_root",
                 addr, port, num_threads);
        let web_dir = Path::new("../../web_root");
        assert!(env::set_current_dir(&web_dir).is_ok());
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

    // TODO(allenpthuang): prettify this section
    let test_100 = b"GET /100 HTTP/1.1\r\n";
    let test_1000 = b"GET /1000 HTTP/1.1\r\n";
    let test_10000 = b"GET /10000 HTTP/1.1\r\n";
    let test_100000 = b"GET /100000 HTTP/1.1\r\n";
    let test_1000000 = b"GET /1000000 HTTP/1.1\r\n";
    let test_10000000 = b"GET /10000000 HTTP/1.1\r\n";


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
        else if buffer.starts_with(test_100) {
            ("HTTP/1.1 200 OK", "100", "application/octet-stream")
        }
        else if buffer.starts_with(test_1000) {
            ("HTTP/1.1 200 OK", "1000", "application/octet-stream")
        }
        else if buffer.starts_with(test_10000) {
            ("HTTP/1.1 200 OK", "10000", "application/octet-stream")
        }
        else if buffer.starts_with(test_100000) {
            ("HTTP/1.1 200 OK", "100000", "application/octet-stream")
        }
        else if buffer.starts_with(test_1000000) {
            ("HTTP/1.1 200 OK", "1000000", "application/octet-stream")
        }
        else if buffer.starts_with(test_10000000) {
            ("HTTP/1.1 200 OK", "10000000", "application/octet-stream")
        }
        else {
            ("HTTP/1.1 404 NOT FOUND", "404.html", "text/html")
        };

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
