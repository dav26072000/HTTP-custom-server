# HTTP Custom Server and Client

## Overview

This project implements a simple custom HTTP-like server and client using C. The server supports basic request types such as GET, POST, and DELETE, and communicates with the client through TCP sockets. It processes requests and sends appropriate responses back to the client.

## Features

- **Server**:
  - Listens on a specified port (`8080` by default).
  - Handles multiple clients concurrently using multithreading.
  - Supports custom HTTP-like requests: `GET`, `POST`, and `DELETE`.
  - Validates request headers (such as `Content-Type` and `Request-Type`).
  - Uses `pthread` for creating threads and `mutex` for synchronization.

- **Client**:
  - Connects to the server and interacts using custom requests.
  - Supports sending `GET`, `POST`, and `DELETE` requests.
  - Allows user input for request type and POST content.

## Files

- **`server.c`**: Contains the implementation of the server that handles incoming client connections and processes HTTP-like requests.
- **`client.c`**: Implements a client that connects to the server and sends requests based on user input.
- **`requests_handlers.h`**: A header file that contains function declarations for handling `GET`, `POST`, and `DELETE` requests.

## How It Works

- **Server**:
  1. The server listens for incoming client connections on port 8080.
  2. For each client, a new thread is created to handle the communication.
  3. The server reads client requests, validates the headers, and processes the request (GET, POST, or DELETE).
  4. It then sends a response based on the type of request.

- **Client**:
  1. The client connects to the server using a TCP connection.
  2. Upon connection, the server sends a menu showing the available request types.
  3. The user can send a request by choosing a request type (GET, POST, DELETE).
  4. If `POST` is selected, the user is prompted to enter content, which is then sent to the server.
  5. The server responds with either success or error messages.

## Setup

### Prerequisites
- Linux-based system
- GCC compiler
- Basic knowledge of C programming and socket programming

### Compilation

To compile both the server and client, run the following commands in the terminal:

```bash
gcc server.c requests_handlers.c -o server -lpthread
gcc client.c -o client
