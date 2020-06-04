#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <ctype.h>

#define NAME 50
#define GRID 2
#define X 0
#define Y 1
#define CLIENT_TAG "client"

int CAPACITY = 30;

typedef enum
{
    false,
    true
} bool;

typedef struct
{
    char name[NAME];
    int location[GRID];
    double speed;
    char **flowers;
    int flowersSize;
    // queue
} Florist;

typedef struct
{
    char name[NAME];
    int location[GRID];
    char flower[NAME];
} Client;

void checkUsage(int argc, char *argv[], char **filePath);
Florist *setFlowers(Florist *florist, char *filePath, int *counterFlorist);
void readStringSnippet(int fp, char *str, int control, char *next);
void printFlorist(Florist *florist, int counterFlorist);
void releaseAllFloristSources(Florist *florist, int counterFlorist);
void performClients(Florist *florist, char *filePath, int counterFlorist);
void goDesiredPointAsString(int fp, char *str, int control);
void getClient(int fp, Client *client);
void printClient(Client *client);

int main(int argc, char *argv[])
{
    char *filePath;
    Florist *florist;
    int counterFlorist = 0;

    checkUsage(argc, argv, &filePath);

    //printf("file: %s\n", filePath);

    florist = setFlowers(florist, filePath, &counterFlorist);

    printFlorist(florist, counterFlorist);

    performClients(florist, filePath, counterFlorist);

    releaseAllFloristSources(florist, counterFlorist);
    free(filePath);

    return 0;
}

void performClients(Florist *florist, char *filePath, int counterFlorist)
{

    int fp;
    Client client;

    fp = open(filePath, O_RDONLY);
    if (fp == -1)
    {
        fprintf(stderr, "%s is not exist\n", filePath);
        exit(0);
    }
    for (int i = 0; i < 24; i++)
    {
        getClient(fp, &client);

        printClient(&client);
    }
}

void getClient(int fp, Client *client)
{

    char str[NAME];
    char next = ' ';

    goDesiredPointAsString(fp, CLIENT_TAG, _ISalpha);

    readStringSnippet(fp, str, _ISalpha, &next);
    sprintf(client->name, "%s", str);
    if (isdigit(next)) {
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
}

void goDesiredPointAsString(int fp, char *str, int control)
{

    char tempStr[NAME];
    char next = ' ';

    do
    {
        readStringSnippet(fp, tempStr, control, &next);
    } while (strcmp(tempStr, str) != 0);

    lseek(fp, -strlen(str) - 1, SEEK_CUR);
}

Florist *setFlowers(Florist *florist, char *filePath, int *counterFlorist)
{
    int fp;
    char str[NAME], strTemp[NAME], next = ' ';

    fp = open(filePath, O_RDONLY);
    if (fp == -1)
    {
        fprintf(stderr, "%s is not exist\n", filePath);
        exit(0);
    }

    florist = (Florist *)malloc(CAPACITY * sizeof(Florist));
    *counterFlorist = 0;
    while (true)
    {
        readStringSnippet(fp, str, _ISalpha, &next);
        if (strcmp(str, CLIENT_TAG) == 0)
            break;
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
            florist[*counterFlorist].flowers[florist[*counterFlorist].flowersSize] = (char *)malloc(CAPACITY * sizeof(char));
            readStringSnippet(fp, str, _ISalpha, &next);
            sprintf(florist[*counterFlorist].flowers[florist[*counterFlorist].flowersSize], "%s", str);
            florist[*counterFlorist].flowersSize++;
        }
        *counterFlorist += 1;
    }
    close(fp);

    return florist;
}

void printClient(Client *client)
{

    fprintf(stderr, "%s ", client->name);
    fprintf(stderr, "%d,%d ", client->location[X], client->location[Y]);
    fprintf(stderr, "%s\n", client->flower);
}

void printFlorist(Florist *florist, int counterFlorist)
{

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
    }
}

void readStringSnippet(int fp, char *str, int control, char *next)
{
    char buffer = ' ';
    int count = 0;

    do
    {
        read(fp, &buffer, sizeof(char));
        if (control == _ISdigit && buffer == '-') {
            str[count++] = '-';
        }
    } while (!__isctype(buffer, control));

    while (__isctype(buffer, control))
    {
        if (__isctype(buffer, control))
        {
            str[count++] = buffer;
        }
        read(fp, &buffer, sizeof(char));
    }

    str[count] = '\0';
    *next = buffer;
}

void releaseAllFloristSources(Florist *florist, int counterFlorist)
{

    for (int i = 0; i < counterFlorist; i++)
    {
        for (int j = 0; j < florist[i].flowersSize; j++)
        {
            free(florist[i].flowers[j]);
        }
        free(florist[i].flowers);
    }
    free(florist);
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