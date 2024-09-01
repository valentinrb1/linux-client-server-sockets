/**
 * @file common.h
 * 
 * @brief Archivo de cabecera para definiciones comunes a todos los archivos fuente.
 * 
 * @details Contiene las definiciones de los tipos de datos utilizados en el programa, las librerías utilizadas y las constantes.
 *
 * @author Robledo, Valentín
 * @date Mayo 2023
 * @version 1.0
 * 
 * @copyright Copyright (c) 2023 
 */

#ifndef __COMMON_H_
#define __COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <zlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "../cJSON/cJSON.h"

/* Unix socket file path*/
#define SOCKET_PATH "/tmp/unix_socket"

/* IPv4 socket port*/
#define SOCKET_PORT_IPV4 3725

/* IPv6 socket port*/
#define SOCKET_PORT_IPV6 3726

/* Data type representing each client type*/
typedef enum client_t
{
    CLIENT_A,
    CLIENT_B,
    CLIENT_C
} client_t;

/* Data type representing each message type*/
typedef enum msg_t
{
    SERVER_MESSAGE,
    CLIENT_MESSAGE
} msg_t;

/* Data type representing the status of a client*/
typedef enum{
    CLIENT_UP,
    CLIENT_DOWN,
} client_status;

/* Data type representing the server status*/
typedef enum{
    SERVER_UP,
    SERVER_DOWN,
} server_status;

#endif // __COMMON_H_ 