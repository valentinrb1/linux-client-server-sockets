/**
 * @file clients.c
 * 
 * @brief Source file for the implementation of the client functions.
 *
 * @details Contains the main function and the functions used by the client.
 * 
 * @author Robledo, Valentín 
 * @date Mayo 2023
 * @version 1.0
 * 
 * @copyright Copyright (c) 2023
 */

#include "../inc/clients.h"

int main(int argc, char* argv[]) 
{   
    if(argc == 3)
    {
        client_t client_type = (client_t)atoi(argv[1]);
        
        client_init(client_type, atoi(argv[2]), NULL);

        message_sender(client_socket, client_type);
    }
    else if(argc == 4)
    {
        client_t client_type = (client_t)atoi(argv[1]);
        
        client_init(client_type, atoi(argv[2]), argv[3]);

        message_sender(client_socket, client_type);
    }
    else
    {
        printf("Error: Cantidad de argumentos inválidos.");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void message_sender(int client_socket, client_t client_type)
{
    create_thread(client_socket, client_type);
    FILE* file = stdin;
    u_int8_t read_flag;

    while(1){   
        char *command;

        if(client_status_f == SENDING)
        {
            do
            {
                command = malloc(100);               
                read_flag = read_input(file, command);

                if(read_flag == INPUT_OMIT)
                    free(command);
            }while(read_flag == INPUT_OMIT);

            send_data(client_socket, command, client_type, CLIENT_MESSAGE);
            free(command);
            client_status_f = RECEIVING;
        }
    }
}

void client_init(client_t client_type, int protocol_type, const char* arg)
{
    server_flag = SERVER_UP;
    client_flag = CLIENT_UP;
    client_status_f = SENDING;

    timeout.tv_sec = 0;
    timeout.tv_usec = 300;

    signal(SIGINT, sigint_handler);
    signal(SIGUSR1, sigusr1_handler);

    switch(protocol_type)
    {
    case 0:
        if(connect_unix_sockect(SOCKET_PATH) == -1)
            exit(EXIT_FAILURE);

        break;

    case 1:
        if(connect_ipv4_sockect(arg, SOCKET_PORT_IPV4) == -1)
            exit(EXIT_FAILURE);

        break;

    case 2:
        if(connect_ipv6_sockect(arg, SOCKET_PORT_IPV6) == -1)
            exit(EXIT_FAILURE);

        break;
    
    default:
        perror("Protocolo inválido.\n");
        exit(EXIT_FAILURE);
        break;
    }

    if(send(client_socket, &client_type, sizeof(client_type), 0) == -1)
    {
        perror("Error al enviar el tipo de cliente\n");
        exit(EXIT_FAILURE);
    }
}

int connect_unix_sockect(const char *socket_path)
{
    struct sockaddr_un server_address;

    unix_socket_path = strdup(socket_path);
    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (client_socket < 0) 
    {
        perror("socket() failed");
        return -1;
    } 

    memset(&server_address, 0, sizeof(server_address));

    server_address.sun_family = AF_UNIX;

    strcpy(server_address.sun_path, unix_socket_path);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("connect() failed");
        return -1;
    }

    return 0;
}

int connect_ipv4_sockect(const char *socket_ipv4, const uint16_t socket_port)
{
    struct sockaddr_in server_address;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        perror("socket() failed");
        return -1;
    } 

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(socket_port);

    if (inet_pton(AF_INET, socket_ipv4, &server_address.sin_addr) <= 0) 
    {
        perror("socket() failed");
        return -1;
    }

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("connect() failed");
        return -1;
    }

    return 0;
}

int connect_ipv6_sockect(const char *socket_ipv6, const uint16_t socket_port)
{
    struct sockaddr_in6 server_address;

    client_socket = socket(AF_INET6, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        perror("socket() failed");
        return -1;
    } 

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin6_family = AF_INET6;
    server_address.sin6_port = htons(socket_port);

    if (inet_pton(AF_INET6, socket_ipv6, &server_address.sin6_addr) <= 0) 
    {
        perror("socket() failed");
        return -1;
    }

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("connect() failed");
        return -1;
    }

    return 0;
}

void create_thread(int client_socket, client_t client_type)
{
    pthread_t tid;
    
    struct thread_args *args = malloc(sizeof(struct thread_args));
    args->client_socket = client_socket;
    args->client_type = client_type;
    args->main_tid = pthread_self();
    
    if(pthread_create(&tid, NULL, &thread_function, (void *)args) != 0)
    {
        printf("Error al crear el hilo.\n");
        exit(EXIT_FAILURE);
    }
}

void *thread_function(void *arg)
{
    struct thread_args *args = (struct thread_args *)arg;
    int client_tsocket = args->client_socket;
    client_t client_type = args->client_type;
    pthread_t main_tid = args->main_tid;
    free(arg);
    
    fd_set read_fds;
    FD_ZERO(&read_fds);

    receiving_logic(client_tsocket, client_type, read_fds);

    pthread_kill(main_tid, SIGUSR1);

    return NULL;
}

void receiving_logic(int client_tsocket, client_t client_type, fd_set read_fds)
{
    while(1)
    {
        FD_ZERO(&read_fds);
        FD_SET(client_tsocket, &read_fds);
        u_int8_t receive_message;   
        char* data;

        int ret = select(client_tsocket + 1, &read_fds, NULL, NULL, &timeout);
        if(ret == 0)
        {
            if(server_flag == SERVER_DOWN)
                break;
        }
        else if(ret > 0)
        {
            ssize_t rec = recv(client_tsocket, &receive_message, sizeof(receive_message), 0);
            
            if(receive_message == SERVER_MESSAGE && client_status_f == RECEIVING && rec > (ssize_t)0)
            {
                data = receive_data(client_tsocket, client_type, SERVER_MESSAGE);
                if(data == NULL)
                    break;
                
                printf("%s\n", data);

                free(data);
                client_status_f = SENDING;
            }
            else if(rec == (ssize_t)0)
                break;
        }   
    }

    return;
}

void close_client(int client_socket)
{   
    //printf("\033[2J\033[1;1H");
    if(server_flag == SERVER_DOWN)
        printf("Pérdida de conexión con el servidor.\n");
    else if(client_flag == CLIENT_DOWN)
        printf("El cliente ha sido desconectado.\n");

    close(client_socket);
    free(unix_socket_path);

    exit(EXIT_SUCCESS);
}

void sigint_handler()
{
    client_flag = CLIENT_DOWN;
    close_client(client_socket);
}

void sigusr1_handler()
{
    server_flag = SERVER_DOWN;
    close_client(client_socket);
}

u_int8_t read_input(FILE* file, char* line)
{
    char* result = fgets(line, 100, file);                                                                 
    if(result == NULL || *line == '\n' || *line == ' ')
        return INPUT_OMIT;
    
    if(line[strlen(line) - 1] != '\n')
        line[strlen(line)] = '\0';
    else
        line[strlen(line) - 1] = '\0';         
    
    for(size_t i = 0; i <= strlen(line) - 1; i++)
        if(line[i] == '\t') 
            line[i] = ' '; 

    return INPUT_OK;
}