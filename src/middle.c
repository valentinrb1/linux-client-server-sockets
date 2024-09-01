/**
 * @file middle.c
 * 
 * @brief Archivo fuente para la implementación de las funciones del middleware.
 * 
 * @details Contiiene funciones utilizadas por el middleware para establecer la comunicación
 * entre los clientes y el servidor.
 * 
 * @author Robledo, Valentín
 * @date Mayo 2023
 * @version 1.0
 * 
 * @copyright Copyright (c) 2023
 */

#include "../inc/middle.h"

void send_data(int client_socket, char* data, client_t client_type, msg_t msg_type)
{     
    size_t data_size = strlen(data);
    size_t num_packets = data_size / PACKET_SIZE + (data_size % PACKET_SIZE == 0 ? 0 : 1);

    data_packet* first_packet = data_packing(data, data_size, num_packets);
    data_packet* current_packet = first_packet;
    
    if(send(client_socket, &num_packets, sizeof(num_packets), 0) == -1)
        send_error_handler("Error: No se pudo enviar el número de paquetes");

    for(size_t i = 0; i < num_packets; i++)
    {
        char* data_packet_json_string = json_format(current_packet);

        if(client_type == CLIENT_B && msg_type == SERVER_MESSAGE)
            send_compress_data(client_socket, data_packet_json_string);
        else
            send_raw_data(client_socket, data_packet_json_string);

        current_packet = current_packet->next;

        free(data_packet_json_string);
    }

    if(msg_type == SERVER_MESSAGE)
        printf("Mensaje enviado al cliente %d de tamaño %ld[Kb].\n", client_socket, data_size);

    free_package_list(first_packet);
}

void send_raw_data(int client_socket, char* data_packet_json_string)
{
    checksum_status checksum_status;

    size_t json_size = strlen(data_packet_json_string);

    if(send(client_socket, &json_size, sizeof(json_size), 0) == -1)
        send_error_handler("Error: No se pudo enviar el tamaño del paquete");

    do{
        if(send(client_socket, data_packet_json_string, json_size, 0) == -1)
            send_error_handler("Error: No se pudo enviar el paquete");
        
        if(recv(client_socket, &checksum_status, sizeof(checksum_status), 0) == -1)
            recv_error_handler("Error: No se pudo recibir el estado del checksum (paquete sin comprimir)");
    }while(checksum_status == CHECKSUM_FAIL);
}

void send_compress_data(int client_socket, char* data_packet_json_string)
{
    checksum_status checksum_status;
    size_t json_size = strlen(data_packet_json_string);

    create_file(data_packet_json_string);

    FILE *file = fopen("../files/data.json.gz", "rb");
    if (file == NULL) {
        perror("Error al abrir archivo");
        exit(EXIT_FAILURE);
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    
    char* buffer = (char*) malloc(sizeof(char) * (size_t)file_size);
    fread(buffer, sizeof(char), (size_t)file_size, file);
    fclose(file);
    
    if(send(client_socket, &file_size, sizeof(file_size), 0) == -1)
        send_error_handler("Error: No se pudo enviar el tamaño del paquete comprimido");

    if(send(client_socket, &json_size, sizeof(json_size), 0) == -1)
        send_error_handler("Error: No se pudo enviar el tamaño del paquete");

    do
    {
        if(send(client_socket, buffer, (sizeof(char) * (size_t)file_size), 0) == -1)
            send_error_handler("Error: No se pudo enviar el paquete comprimido");

        if(recv(client_socket, &checksum_status, sizeof(checksum_status), 0) == -1)
            recv_error_handler("Error: No se pudo recibir el estado del checksum (paquete comprimido)");
    }while(checksum_status == CHECKSUM_FAIL);

    free(buffer);
}

char* receive_data(int client_socket, client_t client_type, msg_t message_type)
{   
    data_packet* first_packet = NULL;
    data_packet* prev_packet = NULL;
    data_packet* current_packet = NULL;
    checksum_status checksum_status;

    size_t num_packets = 0;

    ssize_t rec = recv(client_socket, &num_packets, sizeof(num_packets), 0);
    if(rec == (ssize_t)-1)
        recv_error_handler("Error: No se pudo recibir el número de paquetes");
    else if(rec == (ssize_t)0) //Retorna 0 si el cliente se desconecta.
        return NULL;

    for(size_t i = 0; i < num_packets; i++)
    {  
        size_t json_size = 0;
        char* data_packet_json_string;

        if(client_type == CLIENT_A || client_type == CLIENT_C || (client_type == CLIENT_B && message_type == CLIENT_MESSAGE))
        {
            if(recv(client_socket, &json_size, sizeof(json_size), 0) == -1)
                recv_error_handler("Error: No se pudo recibir el tamaño del paquete");

            data_packet_json_string = calloc(json_size, sizeof(char));
        }
        
        while (1)
        {
            if(client_type == CLIENT_B && message_type == SERVER_MESSAGE)
                data_packet_json_string = receive_compress_data(client_socket);
            else
                if(recv(client_socket, data_packet_json_string, json_size, 0) == -1)
                    send_error_handler("Error: No se pudo recibir el paquete");

            current_packet = calloc(1, sizeof(data_packet));

            json_unformat(data_packet_json_string, current_packet);

            if(first_packet == NULL)
                first_packet = current_packet;
            else
                prev_packet->next = current_packet;

            checksum_status = checksum_check(current_packet, client_socket);

            if(checksum_status == CHECKSUM_FAIL)
            {
                if(prev_packet != NULL)
                    prev_packet->next = NULL;
                else
                    first_packet = NULL;
            }
            else
                break;
        }

        free(data_packet_json_string);

        if(current_packet->flag_last)
            break;
        
        prev_packet = current_packet;
    }

    char* unpacked_data = data_unpacking(first_packet, num_packets);

    free_package_list(first_packet);

    return unpacked_data;
}

void free_package_list(data_packet* first_packet)
{
    data_packet* current_packet = first_packet;
    data_packet* aux;

    while(current_packet != NULL)
    {
        aux = current_packet;
        current_packet = current_packet->next;
        free(aux);
    }
}

char* receive_compress_data(int client_socket)
{
    long file_size;
    size_t json_size;

    if(recv(client_socket, &file_size, sizeof(file_size), 0) == -1)
        recv_error_handler("Error: No se pudo recibir el tamaño del paquete comprimido");
    
    if(recv(client_socket, &json_size, sizeof(json_size), 0) == -1)
        recv_error_handler("Error: No se pudo recibir el tamaño del paquete");

    char *buffer = (char*) malloc(sizeof(char) * (size_t)file_size);

    if(recv(client_socket, buffer, (sizeof(char) * (size_t)file_size), 0) == -1)
        recv_error_handler("Error: No se pudo recibir el paquete comprimido");

    FILE *fp = fopen("../files/data_received.json.gz", "wb");
    if (fp == NULL) {
        perror("Error al abrir archivo");
        exit(EXIT_FAILURE);
    }
    fwrite(buffer, sizeof(char), (size_t)file_size, fp);
    
    fclose(fp);
    free(buffer);

    gzFile gzfp = gzopen("../files/data_received.json.gz", "rb");
    if (gzfp == NULL) {
        perror("Error al abrir archivo gz");
        exit(EXIT_FAILURE);
    }

    char* data_json = malloc(json_size);
    gzread(gzfp, data_json, (uInt)json_size);
    gzclose(gzfp);

    return data_json;
}

data_packet* data_packing(char* data, size_t data_size, size_t num_packets)
{
    data_packet* first_packet = calloc(1, sizeof(data_packet));
    data_packet* last_packet = first_packet;
    first_packet->next = NULL;
    last_packet->flag_last = 0;
    
    for(size_t i = 0; i < num_packets; i++)
    {
        size_t remaining_bytes = data_size - i * PACKET_SIZE;
        size_t packet_bytes = remaining_bytes < PACKET_SIZE - 1 ? remaining_bytes : PACKET_SIZE - 1;

        memcpy(last_packet->data, data + i * PACKET_SIZE, packet_bytes);
        last_packet->data[packet_bytes] = '\0';

        uLong crc_checksum = crc32(0L, Z_NULL, 0);
        last_packet->crc_checksum = crc32(crc_checksum, (const Bytef *)last_packet->data, (uInt)strlen(last_packet->data));
        
        if(remaining_bytes > 0)
        {
            data_packet* new_packet = calloc(1, sizeof(data_packet));
            new_packet->next = NULL;
            last_packet->next = new_packet;
            last_packet = new_packet;
        }
    }

    last_packet->flag_last = 1;

    return first_packet;
}

char* data_unpacking(data_packet* first_packet, size_t num_packets)
{   
    data_packet* current_packet = first_packet;

    char* data = calloc(PACKET_SIZE * num_packets, sizeof(char));

    for(size_t i = 0; i < num_packets; i++)
    {
        strcat(data, current_packet->data);
        current_packet = current_packet->next;
    }

    return data;
}

char* json_format(data_packet* data_packet)
{
    cJSON *data_packet_json = cJSON_CreateObject();

    cJSON_AddStringToObject(data_packet_json, "message", data_packet->data);
    cJSON_AddNumberToObject(data_packet_json, "crc_checksum", (double)data_packet->crc_checksum);
    cJSON_AddNumberToObject(data_packet_json, "flag_last", (double)data_packet->flag_last);
    
    char* data_packet_json_string = cJSON_Print(data_packet_json);
    
    cJSON_Delete(data_packet_json);

    return data_packet_json_string;
}

void json_unformat(char* data_packet_json_string, data_packet* packet)
{
    cJSON *data_packet_json = cJSON_Parse(data_packet_json_string);

    strcpy(packet->data, cJSON_GetObjectItem(data_packet_json, "message")->valuestring);
    packet->crc_checksum = (uLong)cJSON_GetObjectItem(data_packet_json, "crc_checksum")->valuedouble;
    packet->flag_last = (u_int8_t)cJSON_GetObjectItem(data_packet_json, "flag_last")->valuedouble;

    cJSON_Delete(data_packet_json);
}

void create_file(char* data_packet_json_string)
{
    FILE *fp = fopen("../files/data.json.gz", "wb");
    if (fp == NULL) {
        perror("Error al abrir archivo");
        exit(EXIT_FAILURE);
    }

    gzFile gzfp = gzopen("../files/data.json.gz", "wb");
    if (gzfp == NULL) {
        perror("Error al abrir archivo gz");
        exit(EXIT_FAILURE);
    }

    gzwrite(gzfp, data_packet_json_string, (uInt)strlen(data_packet_json_string));
    
    gzclose(gzfp);
    fclose(fp);
}

u_int8_t checksum_check(data_packet *aux, int client_socket)
{
    checksum_status checksum_status;

    uLong crc_checksum = crc32(0L, Z_NULL, 0);
    crc_checksum = crc32(crc_checksum, (const Bytef *)aux->data, (uInt)strlen(aux->data));

    if(crc_checksum != aux->crc_checksum)
        checksum_status = CHECKSUM_FAIL;
    else
        checksum_status = CHECKSUM_OK;

    if(send(client_socket, &checksum_status, sizeof(checksum_status), 0) == -1)
        send_error_handler("Error: No se pudo enviar el estado del checksum");

    return checksum_status;
}

void send_error_handler(const char* error_message)
{
    perror(error_message);
    exit(EXIT_FAILURE);
}

void recv_error_handler(const char* error_message)
{
    perror(error_message);
    exit(EXIT_FAILURE);
}