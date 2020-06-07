#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

// A utility function to create a new linked list node. 
struct QNode* newNode(char *name, char *flower, int location[2], int distance) 
{ 
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode)); 
    temp->distance = distance;
    temp->location[0] = location[0];
    temp->location[1] = location[1];
    sprintf(temp->name, "%s", name);
    sprintf(temp->flower, "%s", flower); 
    temp->next = NULL; 
    return temp; 
} 
  
// A utility function to create an empty queue 
struct Queue* createQueue() 
{ 
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue)); 
    q->front = q->rear = NULL; 
    q->size = 0;
    return q; 
} 
  
// The function to add a key k to q 
void enQueue(struct Queue* q, char *name, char *flower, int location[2], int distance) 
{ 
    // Create a new LL node 
    struct QNode* temp = newNode(name, flower, location, distance); 
  
    // If queue is empty, then new node is front and rear both 
    if (q->rear == NULL) { 
        q->front = q->rear = temp; 
        return; 
    } 
  
    // Add the new node at the end of queue and change rear 
    q->rear->next = temp; 
    q->rear = temp; 

    q->size++;
} 
  
// Function to remove a key from given queue q 
void deQueue(struct Queue* q) 
{ 
    // If queue is empty, return NULL. 
    if (q->front == NULL) 
        return; 
  
    // Store previous front and move front one node ahead 
    struct QNode* temp = q->front; 
  
    q->front = q->front->next; 
  
    // If front becomes NULL, then change rear also as NULL 
    if (q->front == NULL) 
        q->rear = NULL; 
  
    free(temp);
    q->size--;
}

void freeQueue(struct Queue* q) {

    while(q->front != NULL) {
        deQueue(q);
    }
    free(q);
}