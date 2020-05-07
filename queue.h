#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

//https://www.geeksforgeeks.org/queue-linked-list-implementation/

/** @defgroup Queue Queue
 *
 * @{
 *
 * Functions and data structures used for queue
 */


typedef struct Node QNode;

/**
 * @brief The queue node struct
 * 
 */
struct Node{  
    long int key; 
    QNode* next; 
}; 
  
/**
 * @brief The Queue struct
 * 
 */
typedef struct { 
    QNode *front, *rear; 
}Queue; 

/**
 * @brief Creates new queue node
 * 
 * @return              Pointer to new queue node
 */
QNode* newNode(long int k);

/**
 * @brief Creates a new queue
 * 
 * @return              Pointer to new queue
 */
Queue* createQueue();

/**
 * @brief Adds element to queue            
 */
void enQueue(Queue* q, long int k);

/**
 * @brief Pops and returns element from queue
 * 
 * @return              Front node data
 */
long int deQueue(Queue* q);

/**
 * @brief Checks if a queue is empty
 * 
 * @return              True if empty, false if not empty
 */
bool queue_empty(Queue* q);

/**@}*/

#endif
