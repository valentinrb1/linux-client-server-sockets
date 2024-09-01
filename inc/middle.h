/**
 * @file middle.h
 * 
 * @brief Header file corresponding to the middle.c source file.
 *
 * @details Middleware used for communication between client and server.
 * Contains libraries, definitions of functions and structures used in the middle.c source file.
 * 
 * @author Robledo, Valentín
 * @date Mayo 2023
 * @version 1.0
 *  
 * @copyright Copyright (c) 2023
 */

#ifndef __MIDDLE_H__
#define __MIDDLE_H__

#include <sys/stat.h>
#include "common.h"

/* Size of information packet. */
#define PACKET_SIZE 4096

/* Enumeration representing the status of the checksum. */
typedef enum{
    CHECKSUM_OK,
    CHECKSUM_FAIL
} checksum_status;

/**
 * @brief Estructura de paquete de datos.
 * 
 * @param data Arreglo de caracteres que representa la información.
 * @param crc_checksum Checksum de la información.
 * @param flag_last Flag que indica si es el último paquete.
 * @param next Puntero al siguiente paquete.
 */
typedef struct data_packet
{
    char data[PACKET_SIZE];
    uLong crc_checksum;
    u_int8_t flag_last;
    struct data_packet* next;
} data_packet;

/**
 * @brief Function that is responsible for sending a message.
 *
 * Calls the function that is responsible for creating a data packet. Then, in a loop, it formats
 * each data packet to JSON format and sends it. Depending on the type of client,
 * the information is sent compressed or not.
 *
 * @param client_socket File descriptor (fd) of the client socket.
 * @param data Message to send.
 * @param client_type Type of client.
 * @param msg_type Type of message.
 *
 * @note Function used by both clients and servers to communicate.
 *
 * @return void
 */
void send_data(int client_socket, char* data, client_t client_type, msg_t msg_type);

/**
 * @brief Function that is responsible for sending an uncompressed message.
 *
 * First send the JSON packet size, then send the message and wait for a CHECKSUM_OK.
 * Otherwise, resend the message.
 *
 * @param client_socket File descriptor (fd) of the client socket.
 * @param data_packet_json_string Data packet in JSON format.
 *
 * @return void
 */
void send_raw_data(int client_socket, char* data_packet_json_string);

/**
 * @brief Function that is responsible for sending a compressed message.
 *
 * Calls the function that is responsible for creating a compressed file. Sends the size of the compressed file
 * and the size of the JSON packet. Then, sends the compressed file and waits for a CHECKSUM_OK.
 * Otherwise, sends the message again.
 *
 * @param client_socket File descriptor (fd) of the client socket.
 * @param data_packet_json_string Data packet in JSON format.
 *
 * @return void
 */
void send_compress_data(int client_socket, char* data_packet_json_string);

/**
 * @brief Function that is responsible for receiving a message.
 *
 * First, receives the number of packets to be received to handle the loop.
 * Checks whether you are going to receive compressed or uncompressed data depending on the type of client and the type of message.
 * Once the message is received, it calls a function to verify the checksum.
 * Finally, it deformats the JSON and returns the received message.
 *
 * @param client_socket File descriptor (fd) of the client socket.
 * @param client_type Client type.
 * @param message_type Message type.
 *
 * @note Function used by both clients and the server to communicate.
 * @note If it receives a 0 as the number of packets, it means that the client disconnected and returns NULL.
 *
 * @return char* Message received.
 */
char* receive_data(int client_socket, client_t client_type, msg_t message_type);

/**
 * @brief Function that is responsible for receiving a compressed message.
 *
 * Receives the size of the compressed file and the size of the JSON packet.
 * Receives the compressed file and copies it to a new file.
 * Decompresses the file to obtain the JSON packet.
 *
 * @param client_socket File descriptor (fd) of the client socket.
 *
 * @return char* Message in JSON format.
 */
char* receive_compress_data(int client_socket);

/**
 * @brief Function that is responsible for packaging the message.
 *
 * Creates a list of packets to be sent.
 *
 * @param data Message to be packaged.
 * @param data_size Size of the message to be packaged.
 * @param num_packets Number of data packets.
 *
 * @return data_packet* Pointer to the first data packet.
 */
data_packet* data_packing(char* data, size_t data_size, size_t num_packets);

/**
 * @brief Function that unpacks the message.
 *
 * @param packet Pointer to the first data packet.
 * @param num_packets Number of data packets.
 *
 * @return char* Unpacked message.
 */
char* data_unpacking(data_packet* packet, size_t num_packets);

/**
 * @brief Function that formats a data packet to JSON.
 *
 * @param data_packet Pointer to the data packet.
 *
 * @return char* Data packet in JSON format.
 */
char* json_format(data_packet* data_packet);

/**
 * @brief Function that is responsible for unformatting a JSON data packet.
 *
 * @param data_packet_json_string Data packet in JSON format.
 * @param data_packet Pointer to the data packet.
 */
void json_unformat(char* data_packet_json_string, data_packet* data_packet);

/**
 * @brief Function that is responsible for verifying the checksum of a data packet.
 *
 * Sends a message to the client informing whether the checksum is correct or not.
 *
 * @param aux Pointer to the data packet.
 * @param client_socket File descriptor (fd) of the client socket.
 *
 * @return u_int8_t CHECKSUM_OK or CHECKSUM_FAIL.
 */
u_int8_t checksum_check(data_packet* aux, int client_socket);

/**
 * @brief Function that creates a compressed file.
 *
 * @param data_packet_json_string Data packet in JSON format.
 *
 * @return void
 */
void create_file(char* data_packet_json_string);

/**
 * @brief Function that frees the memory used by the data packet list.
 *
 * @param first_packet Pointer to the first data packet.
 *
 * @return void
 */
void free_package_list(data_packet* first_packet);

/**
 * @brief Function that handles an error when using the send() function.
 *
 * @param error_message Error message.
 */
void send_error_handler(const char* error_message);

/**
 * @brief Function that handles an error when using the recv() function.
 *
 * @param error_message
 */
void recv_error_handler(const char* error_message);

#endif // __MIDDLE_H__