# Project Name

This project is a C implementation of a multithreaded chat server.

## Table of Contents

-   [Installation](#installation)
-   [Usage](#usage)
-   [Technologies Used](#technologies-used)

## Installation

Follow these steps to install and set up the multithreaded chat server:

### 1. Clone or Download

You can clone the project using the following command:

```bash
git clone https://github.com/your-username/multithreaded-chat-server.git
```

Alternatively, you can download the ZIP and extract it to your desired location.

### 2. Navigate to the Project Folder

Navigate to the project folder using the terminal:

```bash
cd multithreaded-chat-server
```

### 3. Build the Project

In the terminal, run the following commands to build the project:

#### 3.1. Configure the Project

In the `build` folder, run `cmake ..` to configure the project:

```bash
cmake ..
```

#### 3.2. Compile the Project

```bash
make
```

### 4. Run the Server

```bash
./multithreaded_server
```

## Usage

After successfully building and running the server, clients can connect to it using a suitable chat client. The server will handle multiple clients concurrently through multithreading.

## Technologies Used

1. **C Programming Language** - The core language used for the server implementation.
2. **Multithreading** - Utilized for handling multiple client connections concurrently.
3. **Socket Programming** - For communication between the server and connected clients.
