/**
 * @file server.c
 *
 * @brief Source file for the implementation of the server functions.
 *
 * @details Contains the main function and the functions used by the server.
 * 
 * @author Robledo, Valentín
 * @date April 2023
 * @version 1.0
 * 
 * @copyright Copyright (c) 2023
 */

#include "../inc/server.h"

int main() 
{ 
    server_init();

    while(1)
        wait_client();
     
    return 0;
}

void server_init()
{   
    struct sigaction sa;
    server_flag = SERVER_UP;

    timeout.tv_sec = 0;
    timeout.tv_usec = 300;

    pthread_mutex_init(&lock, NULL);

    if(create_unix_socket(SOCKET_PATH) == -1)
        close_server();
    
    if(create_ipv4_socket(SOCKET_PORT_IPV4) == -1)
        close_server();
    
    if(create_ipv6_socket(SOCKET_PORT_IPV6) == -1)
        close_server();
    
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);
}

int create_unix_socket(const char *socket_path)
{
    struct sockaddr_un server_address;

    server.unix_socket_path = strdup(socket_path);
    server.unix_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if(server.unix_socket_fd < 0) 
    {
        perror("socket() unix failed");
        return -1;
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sun_family = AF_UNIX;

    strcpy(server_address.sun_path, server.unix_socket_path);

    if(bind(server.unix_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("bind() unix failed");
        return -1;
    }

    if(listen(server.unix_socket_fd, 1) < 0) 
    {
        perror("listen() unix failed");
        return -1;
    }

    return 0;
}

int create_ipv4_socket(const uint16_t socket_port)
{
    struct sockaddr_in server_address;

    server.ipv4_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server.ipv4_socket_fd < 0) 
    {
        perror("socket() ipv4 failed");
        return -1;
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(socket_port);

    if(bind(server.ipv4_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("bind() ipv4 failed");
        return -1;
    }

    if(listen(server.ipv4_socket_fd, 1) < 0) 
    {
        perror("listen() ipv4 failed");
        return -1;
    }

    return 0;
}

int create_ipv6_socket(const uint16_t socket_port)
{
    struct sockaddr_in6 server_address;

    server.ipv6_socket_fd = socket(AF_INET6, SOCK_STREAM, 0);

    if(server.ipv6_socket_fd < 0) 
    {
        perror("socket() ipv6 failed");
        return -1;
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_port = htons(socket_port);

    if(bind(server.ipv6_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("bind() ipv6 failed");
        return -1;
    }

    if(listen(server.ipv6_socket_fd, 1) < 0) 
    {
        perror("listen() ipv6 failed");
        return -1;
    }

    return 0;
}

void wait_client()
{
    int client_socket;
    
    while(1)
    {
        fd_set socket_set;
        int max_socket;

        FD_ZERO(&socket_set);

        FD_SET(server.ipv4_socket_fd, &socket_set);
        FD_SET(server.ipv6_socket_fd, &socket_set);
        FD_SET(server.unix_socket_fd, &socket_set);

        max_socket = (server.ipv4_socket_fd > server.ipv6_socket_fd) ? server.ipv4_socket_fd : server.ipv6_socket_fd;
        max_socket = (server.unix_socket_fd > max_socket) ? server.unix_socket_fd : max_socket;

        int ret = select(max_socket + 1, &socket_set, NULL, NULL, &timeout);
        
        if(ret == -1)
        {
            perror("Error en select, espera de conexión de cliente.\n");
            exit(EXIT_FAILURE);
        }
        else if(ret == 0)
        {
            if(server_flag == SERVER_DOWN)
            {
                close_server();
                return;
            }
        }
        else if(ret > 0)
        {
            if(FD_ISSET(server.ipv4_socket_fd, &socket_set))
                client_socket = accept(server.ipv4_socket_fd, NULL, NULL);

            if(FD_ISSET(server.ipv6_socket_fd, &socket_set))
                client_socket = accept(server.ipv6_socket_fd, NULL, NULL);

            if(FD_ISSET(server.unix_socket_fd, &socket_set))
                client_socket = accept(server.unix_socket_fd, NULL, NULL);

            if(client_socket == -1)
            {
                perror("Error al aceptar la conexión del cliente.\n");
                exit(EXIT_FAILURE);
            }
            else if(client_socket > 0)
            {
                create_thread(client_socket);
                break;
            }
        }
    }
}

void create_thread(int client_socket)
{
    pthread_t tid;
    int *client_tsocket = malloc(sizeof(int));
    *client_tsocket = client_socket;

    if(pthread_create(&tid, NULL, &thread_client_handler, (void *)client_tsocket) != 0)
    {
        printf("Error al crear el hilo.\n");
        exit(EXIT_FAILURE);
    }
}

void *thread_client_handler(void *arg)
{  
    int client_tsocket = *(int *)arg;
    free(arg);

    client_t client_type;
    if(recv(client_tsocket, &client_type, sizeof(client_type), 0) == -1)
    {
        perror("Error al recibir el tipo de cliente.\n");
        exit(EXIT_FAILURE);
    }
    printf("Cliente %d tipo %c conectado.\n", client_tsocket, GET_CLIENT_TYPE_LETTER(client_type));

    fd_set read_fds;

    pthread_mutex_lock(&lock);
    add_thread(pthread_self());
    pthread_mutex_unlock(&lock);

    FD_ZERO(&read_fds);
    
    server_logic(client_tsocket, read_fds, client_type);
    
    close(client_tsocket);

    return NULL;
}

void server_logic(int client_tsocket, fd_set read_fds, client_t client_type)
{
    while(1){
        FD_SET(client_tsocket, &read_fds);
        
        int ret = select(client_tsocket + 1, &read_fds, NULL, NULL, &timeout);
        if(ret == -1){
            perror("Error en select, espera de mensaje por parte del cliente.\n");
            exit(EXIT_FAILURE);
        }
        else if(ret == 0)
        {
            if(server_flag == SERVER_DOWN)
                break;
        }
        else if(ret > 0)
        {
            char* command = receive_data(client_tsocket, client_type, CLIENT_MESSAGE);

            if(command == NULL)
            {
                printf("Cliente %d tipo %c desconectado.\n", client_tsocket, GET_CLIENT_TYPE_LETTER(client_type));

                pthread_mutex_lock(&lock);
                rmv_thread(pthread_self());
                pthread_mutex_unlock(&lock);

                break;
            }
            printf("Cliente %d tipo %c envió: %s\n", client_tsocket, GET_CLIENT_TYPE_LETTER(client_type), command);
            
            client_select(client_tsocket, client_type, command);

            free(command);
        }            
    }

    return;
}

void client_select(int client_tsocket, client_t client_type, char* command)
{
    if(send(client_tsocket, &(u_int8_t){SERVER_MESSAGE}, sizeof(u_int8_t), 0) == -1)
    {
        perror("Error al enviar mensaje de aviso al cliente\n");
        exit(EXIT_FAILURE);
    }

    char* result;

    if(client_type == CLIENT_A || client_type == CLIENT_B)
        result = journalctl_execute(command, client_tsocket);
    else if(client_type == CLIENT_C)
        result = sysinfo_execute(command);
    else
    {       
        printf("Error: cliente no válido.\n");
        close_server();

    }
    
    send_data(client_tsocket, result, client_type, SERVER_MESSAGE);
    
    free(result);

    return;
}

char* journalctl_execute(char* command, int client_fd)
{
    FILE *fp;
    char* result;

    char prompt[1024];
    char file_output[128];
    char file_err[128];
       
    sprintf(file_output, "%s_%d.log", JOURNAL_TMP_OUTPUT, client_fd);
    sprintf(file_err, "%s_%d.log", JOURNAL_TMP_ERROR, client_fd);

    sprintf(prompt, "journalctl %s > %s 2> %s", command, file_output, file_err);
    
    fp = popen(prompt, "r");

    if(fp == NULL) 
    {
        result = calloc(strlen(strerror(errno)) + 27, sizeof(char));
        sprintf(result, "Failed to run command: %s", strerror(errno));
        return result;
    }

    pclose(fp);

    if((result = read_file(file_err)) == NULL)
        if((result = read_file(file_output)) == NULL)
            result = NULL;
    
    remove(file_err);
    remove(file_output);

    return result;
}

char* sysinfo_execute(char* command)
{
    struct sysinfo info;
    if(sysinfo(&info) == -1)
    {
        perror("Error al obtener información del sistema.\n");
        exit(EXIT_FAILURE);
    }
    char* result = calloc(60, sizeof(char));
    if (result == NULL) {
        printf("Error: no se pudo asignar memoria para el buffer.\n");
        exit(EXIT_FAILURE);
    }

    if(!strcmp(command, "freeram"))
        snprintf(result, 60, "Memoria ram libre: %lu %s", info.freeram/1024, "KB");
    else if(!strcmp(command, "loads"))
        snprintf(result, 60, "Carga promedio en el último minuto: %lu", info.loads[0]/65536);
    else
        snprintf(result, 60, "Comando no válido.\nComandos disponibles: <freeram> <loads>");
   
    return result;
}

void close_server()
{
    printf("\nCerrando servidor...\n");

    end_threads();

    pthread_mutex_destroy(&lock);

    close(server.unix_socket_fd);
    close(server.ipv4_socket_fd);
    close(server.ipv6_socket_fd);

    unlink(server.unix_socket_path);
    free(server.unix_socket_path);

    printf("Servidor cerrado\n");
    exit(EXIT_SUCCESS);
}

void sigint_handler(int signum)
{
    if(signum == SIGINT)
        server_flag = SERVER_DOWN;

    close_server();
}