/**
 * @file server.h
 * 
 * @brief Header file corresponding to the server.c source file.
 *
 * @details Contains libraries, function definitions, global variables and structures used in the clients.c source file.
 *
 * @author Robledo, Valentín
 * @date Mayo 2023
 * @version 1.0
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/sysinfo.h>
#include <systemd/sd-journal.h>
#include "middle.h"
#include "server_utils.h"

/* Path to the output file of the journalctl execution */
#define JOURNAL_TMP_OUTPUT "/tmp/journal_output"

/* Path to the error file of the journalctl execution */
#define JOURNAL_TMP_ERROR "/tmp/journal_error"

/**
 * @def GET_CLIENT_TYPE_LETTER
 *
 * @brief Macro that returns the identifier letter corresponding to the client type.
 *
 * @param type Client type.
 *
 * @return 'A' 'B' 'C' Corresponding to each client.
 */
#define GET_CLIENT_TYPE_LETTER(type) ((type == CLIENT_A) ? 'A' : (type == CLIENT_B) ? 'B' : (type == CLIENT_C) ? 'C' : '?')

/**
 * @struct server
 *
 * @brief Structure containing server data.
 *
 * @param unix_socket_path UNIX socket path.
 * @param unix_socket_fd UNIX socket file descriptor (fd).
 * @param ipv4_socket_fd IPV4 socket file descriptor (fd).
 * @param ipv6_socket_fd IPV6 socket file descriptor (fd).
 */
struct
{
    char *unix_socket_path;
    int unix_socket_fd;
    int ipv4_socket_fd;
    int ipv6_socket_fd;
} server;

volatile sig_atomic_t server_flag;
pthread_mutex_t lock;
struct timeval timeout;

/**
 * @brief Function that initializes the server.
 *
 * Creating the sockets and preparing to listen. Also, configures the SIGINT signal to be handled.
 *
 * @return void
 */
void server_init();

/**
 * @brief Function that creates the UNIX socket.
 *
 * @param socket_path UNIX socket path.
 *
 * @return Returns if the connection failed.
 */
int create_ipv4_socket(const uint16_t socket_port);

/**
 * @brief Function that is responsible for creating the IPV4 socket.
 *
 * @param socket_port Port of the IPV4 socket.
 *
 * @return Returns if the connection failed.
 */
int create_ipv6_socket(const uint16_t socket_port);

/**
 * @brief Function that is responsible for creating the IPV6 socket.
 *
 * @param socket_port Port of the IPV6 socket.
 *
 * @return Returns if the connection failed.
 */
int create_unix_socket(const char *socket_path);

/**
 * @brief Function that waits for a client to connect.
 *
 * Every certain timeout it checks if the server is closed. When a client connects,
 * a thread is created to handle it.
 *
 * @return void
 */
void wait_client();

/**
 * @brief Function that is responsible for creating a thread to serve a client.
 *
 * @param client_socket File descriptor (fd) of the client socket.
 */
void create_thread(int client_socket);

/**
 * @brief Function that handles the client.
 *
 * The type of client that connected is received. The thread is added to the thread list.
 * And the function that handles the server logic is called.
 *
 * @param arg Pointer to the parameter.
 *
 * @return void*
 */
void *thread_client_handler(void* arg);

/**
 * @brief Function that handles the server logic.
 *
 * Waits to detect an incoming message from the connected client and checks if the client disconnects.
 * If a message is detected, it calls the function that is responsible for receiving this message and then
 * responding. If the client disconnects, the thread is removed from the list and returns.
 * In addition, every certain timeout it checks if the server is closed.
 *
 * @param client_socket File descriptor (fd) of the client socket.
 * @param read_fds Set of file descriptors (fd) of the client.
 * @param client_type Type of client.
 *
 * @return void
 */
void server_logic(int client_socket, fd_set read_fds, client_t client_type);

/**
 * @brief Function that is responsible for selecting the response for the client.
 *
 * This function sends a warning message to the client. Then it calls the function that is responsible for
 * executing the command according to the type of client. Journalctl for client A and B. Sysinfo for client C.
 * Once the response is obtained, it calls the function that is responsible for sending the message.
 *
 * @param client_tsocket File descriptor (fd) of the client socket.
 * @param client_type Type of client.
 * @param command Command sent by the client.
 *
 * @return void
 */
void client_select(int client_tsocket, client_t client_type, char* command);

/**
 * @brief Function that executes the journalctl command.
 *
 * @param command Command sent by the client.
 * @param client_fd File descriptor (fd) of the client socket.
 *
 * @return char* Command response.
 */
char* journalctl_execute(char* command, int client_fd);

/**
 * @brief Function that executes the sysinfo command.
 *
 * @param command Command sent by the client.
 *
 * @return char* Command response.
 *
 * @note Only accepts the "freeram" and "loads" commands.
 */
char* sysinfo_execute(char* command);

/**
 * @brief Function that handles the SIGINT signal.
 *
 * Lowers the server flag and calls the function that closes it.
 *
 * @return void
 */
void sigint_handler();

/**
 * @brief Function that handles closing the server.
 *
 * Calls the function that handles ending threads, destroys the mutex, and closes the socket.
 *
 * @return void
 */
void close_server();

#endif // __SERVER_H__