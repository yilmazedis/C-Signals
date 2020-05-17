#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define CHEF_NUM 6
#define INGREDIENT_NUM 2

typedef enum
{
    false,
    true
} bool;

typedef struct
{
    pthread_t tID;
    char lakeIng[INGREDIENT_NUM];
} Chef;

typedef struct
{
    sem_t mutex;
    Chef chef[CHEF_NUM];
    char delivered[INGREDIENT_NUM];
    bool isOver;
    bool isDelivered;
    bool isCooked;
} ChefsInfo;

void *chefs(void *arg);
void wholeSaller(ChefsInfo *chfsInf);
void setChefsAbsents(ChefsInfo *chfsInf);
int indicateChef(ChefsInfo *chfsInf, pthread_t id);
bool isDeliveredPrecisely(ChefsInfo *chfsInf, int chefID);
void constructChefs(ChefsInfo *chfsInf);
void destructChefs(ChefsInfo *chfsInf);

int main(void)
{
    ChefsInfo chfsInf;

    constructChefs(&chfsInf);

    wholeSaller(&chfsInf);

    destructChefs(&chfsInf);

    return 0;
}

void *chefs(void *arg)
{

    ChefsInfo *chfsInf = (ChefsInfo *)arg;
    unsigned int chefID;

    chefID = indicateChef(chfsInf, pthread_self());

    while (!chfsInf->isOver)
    {

        sem_wait(&chfsInf->mutex); // Lock Entrance

        // fprintf(stderr, "Absent %c, %c %d\n",
        //             chfsInf->delivered[0],
        //             chfsInf->delivered[1], chefID);

        if (isDeliveredPrecisely(chfsInf, chefID))
        {
            fprintf(stderr, "Absent %c, %c %d\n",
                    chfsInf->chef[chefID].lakeIng[0],
                    chfsInf->chef[chefID].lakeIng[1], chefID);

            // sleep((rand() % 5) + 1);
            sleep(1);

            chfsInf->isCooked = true;
        }

        sem_post(&chfsInf->mutex); // Lock Exit
    }

    return NULL;
}

void wholeSaller(ChefsInfo *chfsInf)
{

    char ingredient;
    FILE *fptr;

    // file reading
    fptr = fopen("input.txt", "r");

    if (fptr == NULL)
    {
        printf("Cannot open file \n");
        exit(0);
    }

    while (!feof(fptr))
    {
        chfsInf->isCooked = false;
        fscanf(fptr, "%c", &ingredient);
        if (ingredient != '\n')
        {
            chfsInf->delivered[0] = ingredient;
        }

        fscanf(fptr, "%c", &ingredient);
        if (ingredient != '\n')
        {
            chfsInf->delivered[1] = ingredient;
        }

        fscanf(fptr, "%c", &ingredient);

        while (chfsInf->isCooked == false)
        {
        }
    }

    chfsInf->isOver = true;
}

bool isDeliveredPrecisely(ChefsInfo *chfsInf, int chefID)
{
    if (chfsInf->isDelivered)
    {
        if (chfsInf->delivered[0] == chfsInf->chef[chefID].lakeIng[0])
        {
            if (chfsInf->delivered[1] == chfsInf->chef[chefID].lakeIng[1])
            {
                return true;
            }
        }
        else if (chfsInf->delivered[1] == chfsInf->chef[chefID].lakeIng[0])
        {
            if (chfsInf->delivered[0] == chfsInf->chef[chefID].lakeIng[1])
            {
                return true;
            }
        }
    }

    return false;
}

int indicateChef(ChefsInfo *chfsInf, pthread_t id)
{

    for (int i = 0; i < CHEF_NUM; i++)
    {
        if (chfsInf->chef[i].tID == id)
        {
            return i;
        }
    }

    return false;
}

void constructChefs(ChefsInfo *chfsInf)
{   
    int err;
    chfsInf->isOver = false;
    setChefsAbsents(chfsInf);

    if (sem_init(&(chfsInf->mutex), 1, 1) < 0)
    {
        perror("semaphore initilization");
        exit(0);
    }

    for (int i = 0; i < CHEF_NUM; i++)
    {
        err = pthread_create(&(chfsInf->chef[i].tID), NULL, &chefs, chfsInf);

        if (err != 0)
            fprintf(stderr, "\ncan't create thread :[%s]", strerror(err));
    }
}
void destructChefs(ChefsInfo *chfsInf)
{
    for (int i = 0; i < CHEF_NUM; i++)
    {
        pthread_join(chfsInf->chef[i].tID, NULL);
    }
}

void setChefsAbsents(ChefsInfo *chfsInf)
{

    chfsInf->chef[0].lakeIng[0] = 'W';
    chfsInf->chef[0].lakeIng[1] = 'S';

    chfsInf->chef[1].lakeIng[0] = 'W';
    chfsInf->chef[1].lakeIng[1] = 'F';

    chfsInf->chef[2].lakeIng[0] = 'F';
    chfsInf->chef[2].lakeIng[1] = 'S';

    chfsInf->chef[3].lakeIng[0] = 'M';
    chfsInf->chef[3].lakeIng[1] = 'W';

    chfsInf->chef[4].lakeIng[0] = 'M';
    chfsInf->chef[4].lakeIng[1] = 'F';

    chfsInf->chef[5].lakeIng[0] = 'M';
    chfsInf->chef[5].lakeIng[1] = 'S';
}