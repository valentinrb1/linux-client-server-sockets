/**
 * @file server_utils.h
 * 
 * @brief Header file for functions used by the server.
 *
 * @author Robledo, Valentín
 * @date Mayo 2023
 * @version 1.0
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef __SERVER_UTILS_H__
#define __SERVER_UTILS_H__

#include "common.h"

/**
 * @struct node
 *
 * @brief Node structure for the list of threads.
 *
 * @param tid Thread identifier.
 * @param next Pointer to the next node.
 */
struct node{
    pthread_t tid;
    struct node* next;
};

/**
 * @struct thread_list
 *
 * @brief List structure for threads.
 *
 * @param head Pointer to the first node.
 * @param last Pointer to the last node.
 */
struct thread_list{
    struct node* head;
    struct node* last;
};

/**
 * @brief Function that is responsible for adding threads to the list.
 *
 * @param tid Thread identifier.
 */
void add_thread(pthread_t tid);

/**
 * @brief Function that is responsible for removing threads from the list.
 *
 * @param tid Thread identifier.
 */
void rmv_thread(pthread_t tid);

/**
 * @brief Function that is responsible for ending all threads.
 */
void end_threads();

/**
 * @brief Function that is responsible for reading a file.
 *
 * @param file_name Name of the file.
 * @return Returns a pointer to the contents of the file.
 */
char* read_file(const char* file_name);

#endif // __SERVER_UTILS_H__
