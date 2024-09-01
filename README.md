# Client-Server using Sockets

## Date - 2023

In this project, client-server communication is established using a three-layer architecture, using three types of sockets: Unix socket, IPv4 and IPv6. Where all transmission is implemented in the intermediate layer.

Supporting three types of clients with different functionalities. Each client is a lightweight client, meaning all processing is done on the server side.

### Authors:
- **Robledo, Valentín**

## How to clone this repository?
You can clone this repository to any directory you want using the following command:

```console
git clone https://github.com/valentinrb1/linux-client-server-sockets.git
```
## How to use?
First, we move to the project folder.

```console
cd linux-client-server-sockets
```

Inside the project directory we are going to create a folder called *build*.

```console
mkdir build
```

We move to that folder.
```console
cd build
```

We run the *cmake* command inside the *build* folder.
```console
cmake ..
```

We run the *make* command, to obtain the binary files that are saved inside the *bin* directory.
```console
make
```

To run the server program.
```console
./bin/server
```

To run the clients, the first parameter indicates what type of client we are going to connect to. This parameter can be 0, 1 or 2 for client A, B or C respectively. Then, a second parameter that indicates what type of socket the connection will be made with, this parameter can be 0, 1 or 2 for the unix socket, ipv4 or ipv6 respectively. Finally, a third parameter that corresponds to the IP, depending on whether the connection is made using ipv4 or ipv6.

```console
./bin/clients <client_type> <socket_type> <ip>
```

---
## Operation
As mentioned above, this project consists of a three-layer client-server model where communication is established through a *socket*, either *unix*, *ipv4* or *ipv6* type.

These clients have the characteristic of being *lightweight*, this means that all the heavy processing occurs on the server side. Therefore, the clients are only responsible for entering the command, the server will receive this command, execute it and send the result to the client.

This command is entered through a *cli* (*command-line interface*).

Data transmission is carried out by the middleware layer, the protocol used by this project has the following characteristics:
- Data format: The data is transmitted in a *JSON* format string, this string contains three fields:
- message: Contains the data to be transmitted.
- crc_checksum: Checksum number using the *crc32* algorithm from the *zlib* library.
- flag_last: Flag indicating if it is the last packet.
- Packets: The message is not sent in a single delivery, but is fragmented into packets where the data weighs up to 4Kb. Each packet carries a *crc_checksum*, this allows us to have more precision in case one of these fails.
- Client B: In this case, the server responds with a *json* compressed file using *gzlib*.

The files transmitted by client B will be saved in the **/files** directory within the project. This directory is created by the `cmake ..` command.

### Client types
- Client A: This client sends a command belonging to *journalctl* and receives the result to display it on the screen.
- Client B: Like client A, this client sends a command belonging to *journalctl* but receives the result in a compressed file.
- Client C: This client only works with two commands belonging to *sysinfo*, "*freeram*" and "*loads*" and receives the result to display it on the screen.

### Client layer
This layer is responsible for the main functionalities corresponding to the client.

When running the program, it receives as an argument the type of client to initialize, this argument can be 0, 1 or 2, to refer to Client A, B or C respectively.

First it will connect to the server through the *socket*, when making the connection it sends a message to let you know what type of client it is.

Once connected, the program is parallelized using two threads.
- Main thread: It is responsible for receiving keyboard input from the user. Once a command is entered, it is sent to the server. It also manages two types of signals:
1. SIGUSR1: If it receives this signal, it means that the secondary thread detected that the server was closed, therefore, it will be responsible for terminating the client.
2. SIGINT: This signal will be received when the user presses the key combination `Ctrl+C`, this means that the client will be closed.
- Secondary thread: It is responsible for waiting for incoming messages from the server and also for checking if the server has been closed. If it detects incoming information, it receives it and displays it on the screen. On the other hand, if it detects that the server has been closed, it sends a SIGUSR1 type signal to the main thread and ends the current thread.

### Middle Layer - Middleware
This layer is responsible for all communication between the client and the server.

Here we can find different functions:
- `send_data()`: Function used to send messages.
- `send_raw_data()`: Function called by `send_data()` as long as compressed information does not need to be sent.
- `send_compress_data()`: Like the previous function, this function will be called by `send_data()` when we need to send compressed information.
- `receive_data()`: Function used to receive messages.
- `receive_compress_data()`: Function called by `receive_data()` when the information to be received is compressed.
- `data_packing()`: Function used to pack data.
- `data_unpacking()`: Function used to unpack data.
- `json_format()`: Function used to format a data packet to *json* format.
- `json_unformat()`: Function used to unformat a *json* and obtain the data.
- `create_file()`: Function used to create a compressed file.
- `checksum_check()`: Function used to check if the checksum matches the data received.

### Server layer
This layer is responsible for the main functionalities corresponding to the server.

When the server is running, it will wait for a client connection. If a client connects to it, the server will be parallelized, a thread will be created for each client that connects to the server. The logic is as follows:
- Main thread: This thread will be in charge of continuing to wait for the connection of more clients.
- Secondary thread: First, it receives what type of client connected. Then, it will wait for the messages that this client sends, once it receives one, it will execute the command received and respond to the client. In addition, it will check if the client closes its connection and if the server itself closes.

Each thread created by a client is saved in a thread list, this serves to ensure that when closing the server we make sure that all the threads are finished.

---
## Licencia

GNU General Public License v3.0 Copyright (c) Robledo, Valentín
