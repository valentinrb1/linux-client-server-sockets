/**
 * @file server_utils.c
 *
 * @brief File of functions used by the server.
 *
 * @details Contains useful functions for the operation of the server.
 * 
 * @author Robledo, Valentín
 * @date April 2023
 * @version 1.0
 * 
 * @copyright Copyright (c) 2023
 */

#include "../inc/server_utils.h"

static struct thread_list thread_list;

char* read_file(const char* file)
{
    FILE *fp = fopen(file, "r");
    unsigned long size;
    char* result;

    if (fp == NULL)
        return NULL;

    fseek(fp, 0, SEEK_END);

    size = (unsigned long) ftell(fp);

    if(size < 1)
    {
        fclose(fp);
        return NULL;
    }

    rewind(fp);

    result = calloc(size, sizeof(char));

    fread(result, 1, size, fp);

    result[size - 1] = '\0';

    fclose(fp);

    return result;
}

void add_thread(pthread_t tid)
{
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    new_node->tid = tid;
    new_node->next = NULL;

    if(thread_list.head == NULL)
    {
        thread_list.head = new_node;
        thread_list.last = new_node;
    }
    else
    {
        thread_list.last->next = new_node;
        thread_list.last = new_node;
    }
}

void rmv_thread(pthread_t tid)
{
    struct node* aux = thread_list.head;
    struct node* prev = NULL;

    while(aux != NULL)
    {
        if(aux->tid == tid)
        {
            if(prev == NULL)
                thread_list.head = aux->next;
            else
                prev->next = aux->next;

            free(aux);
            break;
        }

        prev = aux;
        aux = aux->next;
    }
}

void end_threads()
{
    struct node* aux = thread_list.head;

    while(aux != NULL)
    {
        pthread_join(aux->tid, NULL);
        aux = aux->next;
    }
}