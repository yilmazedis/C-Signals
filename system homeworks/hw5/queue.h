// https://www.geeksforgeeks.org/queue-linked-list-implementation/

// A linked list (LL) node to store a queue entry
struct QNode
{
    int distance;
    char name[50];
    char flower[50];
    int location[2];

    struct QNode *next;
};

// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct Queue
{
    int size;
    struct QNode *front, *rear;
};

// A utility function to create a new linked list node. 
struct QNode* newNode(char *name, char *flower, int location[2], int distance);

// A utility function to create an empty queue 
struct Queue* createQueue();

// The function to add a key k to q 
void enQueue(struct Queue* q, char *name, char *flower, int location[2], int distance);

// Function to remove a key from given queue q 
void deQueue(struct Queue* q);

// free queue;
void freeQueue(struct Queue* q);