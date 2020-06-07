#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <ctype.h>
#include "queue.h"

#define NAME 50
#define POINT 2
#define X 0
#define Y 1
#define CLIENT_TAG "client"
#define SECOND 1000

typedef enum
{
    false,
    true
} bool;

typedef struct
{
    char name[NAME];
    int location[POINT];
    double speed;
    char **flowers;
    int flowersSize;
    struct Queue *queue;
    pthread_mutex_t mutex;
    pthread_cond_t condvar;
    pthread_t tID;
    int numberOfsales;
    int totalTime;
} Florist;

typedef struct
{
    char name[NAME];
    int location[POINT];
    char flower[NAME];
} Client;

int CAPACITY = 30;
volatile int isLive = 1;

void checkUsage(int argc, char *argv[], char **filePath);
Florist *setFlowers(Florist *florist, char *filePath, int *counterFlorist);
void readStringSnippet(int fp, char *str, int control, char *next);
void printFlorist(Florist *florist, int counterFlorist);
void releaseAllFloristSources(Florist *florist, int counterFlorist);
void performClients(Florist *florist, char *filePath, int counterFlorist);
bool goDesiredPointAsString(int fp, char *str, int control);
bool getClient(int fp, Client *client);
void printClient(Client *client);
int chebyshevDistanceBetweenTwoPoints(int location1[POINT], int location2[POINT]);
int getClosestFlorist(Client *client, Florist *florist, int counterFlorist, int *distance);
void constructFloristTable(Florist *florist, int counterFlorist);
void *floristTable(void *arg);
void destructFloristTable(Florist *florist, int counterFlorist);
void printOneFlorist(Florist *florist);
void  pretyPrintAfterAllRequestProcessed(Florist *florist, int counterFlorist);
int MAX(int A, int B);
int MIN(int A, int B);
int ABS(int A);

int main(int argc, char *argv[])
{
    char *filePath;
    Florist *florist;
    int counterFlorist = 0;

    checkUsage(argc, argv, &filePath);

    //printf("file: %s\n", filePath);

    // set florist
    florist = setFlowers(florist, filePath, &counterFlorist);

    
    // print
    //printFlorist(florist, counterFlorist);

    ///////////////////// Actual Process Region
    // construct Florist
    constructFloristTable(florist, counterFlorist);

    // Perform Florist
    performClients(florist, filePath, counterFlorist);
    /////////////////////

    // print
    // printFlorist(florist, counterFlorist);
    // printFlorist(florist, counterFlorist);
    //fprintf(stderr, "%d\n", counterFlorist);

    // Release All Sources & Power Off System.
    destructFloristTable(florist, counterFlorist);

    pretyPrintAfterAllRequestProcessed(florist, counterFlorist);

    releaseAllFloristSources(florist, counterFlorist);
    free(filePath);

    return 0;
}

void *floristTable(void *arg)
{

    Florist *florist = (Florist *)arg;
    int prepration_time = 0;

    // printOneFlorist(florist);

    while (isLive || florist->queue->size > 0)
    {
        pthread_mutex_lock(&(florist->mutex));

        while (florist->queue->size == 0 && isLive)
        {
            // fprintf(stderr,"sdsd");
            pthread_cond_wait(&(florist->condvar), &(florist->mutex));
        }

        if (florist->queue->size > 0) {
            prepration_time = (rand() % 251) * SECOND;
            usleep(prepration_time); // equal to the time of preparation + time of delivery
            florist->totalTime += florist->queue->front->distance + prepration_time;
            fprintf(stderr, "Florist %s has delivered a %s to %s in %02dms\n", florist->name, florist->queue->front->flower, florist->queue->front->name, florist->queue->front->distance + prepration_time);
            
            //fprintf(stderr, "thread: %ld %s\n", florist->tID % 100, florist->queue->front->name);

            deQueue(florist->queue);
        }

        pthread_mutex_unlock(&(florist->mutex));
    }

    fprintf(stderr, "%s closing shop.\n", florist->name);
    
    pthread_exit(NULL);
}

void performClients(Florist *florist, char *filePath, int counterFlorist)
{

    int fp;
    Client client;
    int floristId = -1, distance;

    fp = open(filePath, O_RDONLY);
    if (fp == -1)
    {
        fprintf(stderr, "%s is not exist\n", filePath);
        exit(0);
    }
    while (true)
    {
        if (getClient(fp, &client))
        {
            break;
        }
        // printClient(&client);

        floristId = getClosestFlorist(&client, florist, counterFlorist, &distance);
        florist[floristId].numberOfsales += 1;
        pthread_mutex_lock(&(florist[floristId].mutex));
        enQueue(florist[floristId].queue, client.name, client.flower, client.location, distance);

        // printClient(&client);
        // fprintf(stderr, "%d\n", floristId);

        pthread_mutex_unlock(&(florist[floristId].mutex));

        pthread_cond_signal(&(florist[floristId].condvar));
    }
    isLive = 0;

    for (int i = 0; i < counterFlorist; i++)
    {
        pthread_cond_signal(&(florist[i].condvar));
    }

    fprintf(stderr, "All requests processed.\n");
    close(fp);
}

void  pretyPrintAfterAllRequestProcessed(Florist *florist, int counterFlorist) {
    
    fprintf(stderr, "-------------------------------------------------\n");
    fprintf(stderr, "Florist \t\t # of sales \t\t Total time\n");

    for (int i = 0; i < counterFlorist; i++)
    {
        fprintf(stderr, "%-10s \t\t %-10d \t\t %-10dms\n", florist[i].name, florist[i].numberOfsales, florist[i].totalTime);
    }
    fprintf(stderr, "-------------------------------------------------\n");

}

void constructFloristTable(Florist *florist, int counterFlorist)
{

    fprintf(stderr, "Processing requests\n");
    for (int i = 0; i < counterFlorist; i++)
    {
        pthread_create(&(florist[i].tID), NULL, &floristTable, &florist[i]);
    }
}

void destructFloristTable(Florist *florist, int counterFlorist)
{
    for (int i = 0; i < counterFlorist; i++)
    {
        pthread_join(florist[i].tID, NULL);
    }
}

int getClosestFlorist(Client *client, Florist *florist, int counterFlorist, int *distance)
{

    int min = __INT32_MAX__;
    int florisytId = -1;

    for (int i = 0; i < counterFlorist; i++)
    {
        for (int j = 0; j < florist[i].flowersSize; j++)
        {
            if (strcmp(florist[i].flowers[j], client->flower) == 0)
            {
                *distance = chebyshevDistanceBetweenTwoPoints(florist[i].location, client->location);
                min = MIN(min, *distance);
                florisytId = i;
                break;
            }
        }
    }
    return florisytId;
}

int chebyshevDistanceBetweenTwoPoints(int location1[POINT], int location2[POINT])
{
    return MAX(ABS(location1[X] - location1[Y]), ABS(location2[X] - location2[Y]));
}

bool getClient(int fp, Client *client)
{

    char str[NAME] = "";
    char next = ' ';

    if (goDesiredPointAsString(fp, CLIENT_TAG, _ISalpha))
        return true;

    readStringSnippet(fp, str, _ISalpha, &next);
    sprintf(client->name, "%s", str);
    if (isdigit(next))
    {
        lseek(fp, -1, SEEK_CUR);
        readStringSnippet(fp, str, _ISdigit, &next);
        strcat(client->name, str);
    }

    readStringSnippet(fp, str, _ISdigit, &next);
    sscanf(str, "%d", &client->location[X]);

    readStringSnippet(fp, str, _ISdigit, &next);
    sscanf(str, "%d", &client->location[Y]);

    readStringSnippet(fp, str, _ISalpha, &next);
    sprintf(client->flower, "%s", str);

    return false;
}

bool goDesiredPointAsString(int fp, char *str, int control)
{
    char tempStr[NAME] = "";
    char next = ' ';

    do
    {
        readStringSnippet(fp, tempStr, control, &next);
    } while (strcmp(tempStr, str) != 0 && next != 'e');

    lseek(fp, -strlen(str) - 1, SEEK_CUR);
    if (next != 'e')
        return false;
    return true;
}

Florist *setFlowers(Florist *florist, char *filePath, int *counterFlorist)
{
    int fp;
    char str[NAME] = " ", strTemp[NAME] = " ", next = ' ';

    fp = open(filePath, O_RDONLY);
    if (fp == -1)
    {
        fprintf(stderr, "%s is not exist\n", filePath);
        exit(0);
    }

    fprintf(stderr, "Florist application initializing from file: %s\n", filePath);

    florist = (Florist *)malloc(CAPACITY * sizeof(Florist));
    *counterFlorist = 0;
    while (true)
    {
        
        readStringSnippet(fp, str, _ISalpha, &next);
        if (strcmp(str, CLIENT_TAG) == 0) {
            //*counterFlorist -= 1;
            break;
        }

        florist[*counterFlorist].numberOfsales = 0;
        florist[*counterFlorist].totalTime = 0;
        pthread_mutex_init(&florist[*counterFlorist].mutex, NULL);
        pthread_cond_init(&florist[*counterFlorist].condvar, NULL);

        sprintf(florist[*counterFlorist].name, "%s", str);

        readStringSnippet(fp, str, _ISdigit, &next);
        sscanf(str, "%d", &florist[*counterFlorist].location[X]);
        readStringSnippet(fp, str, _ISdigit, &next);
        sscanf(str, "%d", &florist[*counterFlorist].location[Y]);

        readStringSnippet(fp, str, _ISdigit, &next);
        strcat(str, ".");
        readStringSnippet(fp, strTemp, _ISdigit, &next);
        strcat(str, strTemp);
        sscanf(str, "%lf", &florist[*counterFlorist].speed);

        florist[*counterFlorist].flowers = (char **)malloc(CAPACITY * sizeof(char *));
        florist[*counterFlorist].flowersSize = 0;
        while (next != '\n')
        {
            if (florist[*counterFlorist].flowersSize > CAPACITY)
            { // make it more usefull
                florist[*counterFlorist].flowers = (char **)realloc(florist[*counterFlorist].flowers, 2 * CAPACITY * sizeof(char *));
                CAPACITY *= 2;
            }
            florist[*counterFlorist].flowers[florist[*counterFlorist].flowersSize] = (char *)malloc(CAPACITY * sizeof(char));
            readStringSnippet(fp, str, _ISalpha, &next);
            sprintf(florist[*counterFlorist].flowers[florist[*counterFlorist].flowersSize], "%s", str);
            florist[*counterFlorist].flowersSize++;
        }
        florist[*counterFlorist].queue = createQueue();

        *counterFlorist += 1;
    }
    close(fp);
    //*counterFlorist -= 1;

    fprintf(stderr, "%d florists have been created\n", *counterFlorist);

    return florist;
}

void printClient(Client *client)
{

    fprintf(stderr, "%s ", client->name);
    fprintf(stderr, "%d,%d ", client->location[X], client->location[Y]);
    fprintf(stderr, "%s\n", client->flower);
}

void printOneFlorist(Florist *florist)
{
    struct QNode *q;

    fprintf(stderr, "%s ", florist->name);
    fprintf(stderr, "%d,%d ", florist->location[X], florist->location[Y]);
    fprintf(stderr, "%lf ", florist->speed);

    for (int j = 0; j < florist->flowersSize; j++)
    {
        fprintf(stderr, "%s ", florist->flowers[j]);
    }
    fprintf(stderr, "\n");

    q = florist->queue->front;

    while (q)
    {
        fprintf(stderr, "   %s (%d,%d) %s %d\n", q->name, q->location[X], q->location[Y], q->flower, q->distance);
        q = q->next;
    }
}

void printFlorist(Florist *florist, int counterFlorist)
{
    struct QNode *q;
    for (int i = 0; i < counterFlorist; i++)
    {
        fprintf(stderr, "%s ", florist[i].name);
        fprintf(stderr, "%d,%d ", florist[i].location[X], florist[i].location[Y]);
        fprintf(stderr, "%lf ", florist[i].speed);

        for (int j = 0; j < florist[i].flowersSize; j++)
        {
            fprintf(stderr, "%s ", florist[i].flowers[j]);
        }
        fprintf(stderr, "\n");

        q = florist[i].queue->front;

        while (q)
        {
            fprintf(stderr, "   %s (%d,%d) %s %d\n", q->name, q->location[X], q->location[Y], q->flower, q->distance);
            q = q->next;
        }
    }
}

void readStringSnippet(int fp, char *str, int control, char *next)
{
    char buffer = ' ';
    int count = 0;

    do
    {
        if (read(fp, &buffer, sizeof(char)) <= 0)
        {
            *next = 'e';
            return;
        }

        if (control == _ISdigit && buffer == '-')
        {
            str[count++] = '-';
        }
    } while (!__isctype(buffer, control));

    while (__isctype(buffer, control))
    {
        if (__isctype(buffer, control))
        {
            str[count++] = buffer;
        }
        if (read(fp, &buffer, sizeof(char)) <= 0)
        {
            *next = 'e';
            return;
        }
    }

    str[count] = '\0';
    *next = buffer;
}

void releaseAllFloristSources(Florist *florist, int counterFlorist)
{

    for (int i = 0; i < counterFlorist; i++)
    {
        freeQueue(florist[i].queue);
        // fprintf(stderr, " sd %d\n", florist[i].flowersSize);
        for (int j = 0; j < florist[i].flowersSize; j++)
        {
            free(florist[i].flowers[j]);
        }
        free(florist[i].flowers);

        pthread_mutex_destroy(&(florist[i].mutex));
        pthread_cond_destroy(&(florist[i].condvar));
    }
    // so important
    free(florist);
}

int MAX(int A, int B)
{
    return ((A) > (B) ? (A) : (B));
}
int MIN(int A, int B)
{
    return ((A) < (B) ? (A) : (B));
}
int ABS(int A)
{
    return ((A) < 0 ? -(A) : (A));
}

void checkUsage(int argc, char *argv[], char **filePath)
{
    int opt;

    if (argc != 3)
    {
        fprintf(stderr, "Ooooooppppppsssss! You entered wrong arguments. Usage: \n");
        fprintf(stderr, "./program -i filePath\n");
        exit(1);
    }
    while ((opt = getopt(argc, argv, ":i:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            *filePath = (char *)malloc(strlen(optarg) + 1);
            sprintf(*filePath, "%s", optarg);

            break;
        case ':':
        case '?':
            fprintf(stderr, "Ooooooppppppsssss! You entered wrong arguments. Usage: \n");
            fprintf(stderr, "./program -i filePath\n");
            exit(1);
            break;
        }
    }
}
