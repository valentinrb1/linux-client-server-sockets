/**
 * @file clients.h
 * 
 * @brief Header file corresponding to the clients.c source file.
 *
 * @details Contains extra libraries to those included in common.h, function definitions,
 * global variables and structures used in the clients.c source file.
 * 
 * @author Robledo, Valentín
 * @date Mayo 2023
 * @version 1.0
 * 
 * @copyright Copyright (c) 2023
 */
#ifndef __CLIENTS_H__
#define __CLIENTS_H__

#include <stdint.h>
#include "middle.h"

/* Keyboard input was successful */
#define INPUT_OK   0

/* Keyboard input is skipped */
#define INPUT_OMIT 1

/* Client is sending a message */
#define SENDING    0

/* Client is receiving a message */
#define RECEIVING  1

/**
 * @struct thread_args
 *
 * @brief Structure containing the arguments passed to the thread.
 *
 * @param client_socket File descriptor (fd) of the client socket.
 * @param client_type Type of client.
 * @param main_tid Main thread identifier.
 */
struct thread_args
{
    int client_socket;
    client_t client_type;
    pthread_t main_tid;
};

volatile sig_atomic_t server_flag, client_flag, client_status_f;
int client_socket;
char* unix_socket_path;
struct sockaddr_un server_address;
struct timeval timeout;

/**
 * @brief Function that initializes the client.
 *
 * Creating the socket, connecting to the server and sending a first message indicating
 * the type of client it is. In addition, it configures the SIGINT and SIGUSR1 signals to be handled.
 *
 * @param client_type Type of client.
 * @param protocol_type Type of protocol.
 * @param arg if protocol_type = ipv4 or ipv6 => arg = IP, else => arg = NULL.
 *
 * @return void
 */
void client_init(client_t client_type, int protocol_type, const char* arg);

/**
 * @brief Function that creates a unix socket.
 *
 * @param socket_path Unix socket path.
 *
 * @return Returns if the connection failed.
 */
int connect_unix_sockect(const char *socket_path);

/**
 * @brief Function that creates an ipv4 socket.
 *
 * @param socket_ipv4 IPv4 address of the socket.
 * @param socket_port Socket port.
 *
 * @return Returns if the connection failed.
 */
int connect_ipv4_sockect(const char *socket_ipv4, const uint16_t socket_port);

/**
 * @brief Function that creates an ipv6 socket.
 *
 * @param socket_ipv6 IPv6 address of the socket.
 * @param socket_port Socket port.
 *
 * @return Returns whether the connection failed.
 */
int connect_ipv6_sockect(const char *socket_ipv6, const uint16_t socket_port);

/**
 * @brief Function responsible for sending messages to the server.
 *
 * Waits for keyboard input from the user and then sends the message.
 *
 * @param client_socket File descriptor (fd) of the client socket.
 * @param client_type Client type.
 *
 * @return void
 */
void message_sender(int client_socket, client_t client_type);

/**
 * @brief Function that creates a thread.
 *
 * Creates a thread that executes the thread_function.
 *
 * @param client_socket File descriptor (fd) of the client socket.
 * @param client_type Type of client.
 *
 * @return void
 */
void create_thread(int client_socket, client_t client_type);

/**
 * @brief Function that controls the thread.
 *
 * @param arg Pointer to a structure of type thread_args.
 *
 * @return void
 */
void *thread_function(void *arg);

/**
 * @brief Function that is responsible for receiving messages.
 *
 * Waits for a message from the server and prints it on the screen. In addition, it terminates
 * the client in case the server has disconnected.
 *
 * @param client_tsocket File descriptor (fd) of the client socket.
 * @param client_type Client type.
 * @param read_fds Set of file descriptors (fd) of the client sockets.
 *
 * @return void
 */
void receiving_logic(int client_tsocket, client_t client_type, fd_set read_fds);

/**
 * @brief Function that is responsible for disconnecting a client.
 *
 * Prints a message on the screen indicating whether the disconnection was by the client or
 * the server.
 * 
 * @param client_socket File descriptor (fd) of the client socket.
 * 
 * 
 * @return void 
 */
void close_client(int client_socket);

/**
 * @brief SIGINT type signal handler.
 *
 * @return void
 */
void sigint_handler();

/**
 * @brief SIGUSR1 type signal handler.
 *
 * @return void
 */
void sigusr1_handler();

/**
 * @brief Function that reads keyboard input.
 *
 * Prints a message on the screen indicating whether the disconnection was by the client or
 * the server.
 *
 * @param file Name of the file to read.
 * @param line Buffer where the entered line will be written.
 *
 * @return Returns INPUT_OK if the keyboard input was successful, INPUT_OMIT if the keyboard input is omitted.
 */
uint8_t read_input(FILE* file, char* line);

#endif // __CLIENTS_H__