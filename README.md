# SocketSphere – Multithreaded Chat Server

## Overview

SocketSphere is a multithreaded chat server and client application written in modern C++ (C++17). It allows multiple clients to communicate in real-time over TCP sockets with thread-safe messaging.

## Features

- Multi-client chat server
- Thread-safe message handling
- Private messaging and user commands
- Error handling and logging
- Command-line interface for both server and client

## Tech Stack

- C++17
- STL (`<thread>`, `<mutex>`, `<condition_variable>`)
- TCP sockets (`asio` or `boost::asio`)
- Optional: ncurses for client UI enhancements

## Installation

### Prerequisites

- C++17 compatible compiler
- CMake (optional)
- asio or boost::asio library

### Steps

1. Clone the repository:

   ```bash
   git clone https://github.com/jatindalal/SocketSphere.git
   cd SocketSphere
   ```

2. Build the project:

   ```bash
   cmake . -B build
   cmake --build build
   ```

## Usage

### Server

```bash
./SocketSphereServer <port>
```

- Example: `./SocketSphereServer 8080`

### Client

```bash
./SocketSphereClient <server_ip> <port> <username>
```

- Example: `./SocketSphereClient 127.0.0.1 8080 Alice`

### Commands

- `/list` – Show connected users
- `/msg <user> <message>` – Private message
- `/exit` – Disconnect client

## Project Structure

```
/src       -> Source code
/include   -> Header files
/tests     -> Unit and integration tests
/docs      -> Screenshots, demo GIFs
/examples  -> Demo usage scripts
CMakeLists.txt / Makefile
README.md
```

## Demo

- Include screenshots or GIFs of server and client running here.

## Roadmap / To-Do

- Add encrypted messaging
- GUI client using Qt or SFML
- Logging enhancements
- Support for chat rooms / channels

## Contributing

1. Fork the repo
2. Create a branch for your feature: `git checkout -b feature-name`
3. Commit your changes: `git commit -m 'Add new feature'
4. Push to your branch: `git push origin feature-name`
5. Create a pull request

## License

[MIT License](LICENSE)

---

_SocketSphere – A robust and fast C++ chat server for learning and demonstration purposes._
