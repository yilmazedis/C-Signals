#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>

#define CHEF_NUM 6
#define INGREDIENT_NUM 2
#define INGREDIENT_NAME_MAX_SIZE 10

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
    char *delivered;
    bool isOver;
    bool isCooked;
} ChefsInfo;

void *chefs(void *arg);
void operateWholeSaller(ChefsInfo *chfsInf, int fptr);
void setChefsAbsents(ChefsInfo *chfsInf);
int indicateChef(ChefsInfo *chfsInf, pthread_t id);
bool isDeliveredPrecisely(ChefsInfo chfsInf, int chefID);
void constructChefs(ChefsInfo *chfsInf);
void destructChefs(ChefsInfo *chfsInf);
void extendIngredientName(char ingredient, char actualName[INGREDIENT_NAME_MAX_SIZE]);
void checkUsage(int argc, char *argv[], char **filePath);
int checkRequirements(char *filePath);

int main(int argc, char *argv[])
{
    ChefsInfo chfsInf;
    char *filePath;
    int fptr;

    checkUsage(argc, argv, &filePath);

    if(!checkRequirements(filePath)) {
        fprintf(stderr ,"The file include less than 10 line\n");
        exit(0);
    }
    // file reading
    fptr = open(filePath, O_RDONLY);

    if (fptr == -1)
    {
        fprintf(stderr ,"There is not a file\n");
        exit(0);
    }

    constructChefs(&chfsInf);

    operateWholeSaller(&chfsInf, fptr);

    destructChefs(&chfsInf);

    close(fptr);
    free(filePath);

    return 0;
}

int checkRequirements(char *filePath) {
    
    int fptr;
    char buff[INGREDIENT_NUM + 1];
    int i = 0;

    // file reading
    fptr = open(filePath, O_RDONLY);

    if (fptr == -1)
    {
        printf("There is not a file\n");
        exit(0);
    }

    while (read(fptr , buff, 3 * sizeof(char)) != 0) {
        i++;
    }

    close(fptr);
    
    if (i < 10)
        return false;
    else
        return true;
}

void *chefs(void *arg)
{

    ChefsInfo *chfsInf = (ChefsInfo *)arg;
    unsigned int chefID;
    char actualName[INGREDIENT_NAME_MAX_SIZE] = "";
    char actualName2[INGREDIENT_NAME_MAX_SIZE] = "";

    chefID = indicateChef(chfsInf, pthread_self());

    while (!chfsInf->isOver)
    {
        extendIngredientName(chfsInf->chef[chefID].lakeIng[0], actualName);
        extendIngredientName(chfsInf->chef[chefID].lakeIng[1], actualName2);
        fprintf(stderr, "chef%d is waiting for %s and %s\n", chefID + 1, actualName, actualName2);

        sem_wait(&chfsInf->mutex); // Lock Entrance
        if (isDeliveredPrecisely(*chfsInf, chefID))
        {
            extendIngredientName(chfsInf->chef[chefID].lakeIng[0], actualName);
            fprintf(stderr, "chef%d has taken the %s\n", chefID + 1, actualName);

            extendIngredientName(chfsInf->chef[chefID].lakeIng[1], actualName);
            fprintf(stderr, "chef%d has taken the %s\n", chefID + 1, actualName);

            //sleep((rand() % 5) + 1); // actual cal !!!
            sleep(1);

            chfsInf->isCooked = true;

            fprintf(stderr, "chef%d has delivered the dessert to the wholesaler\n", chefID + 1);
        }
        sem_post(&chfsInf->mutex); // Lock Exit
    }

    pthread_exit(NULL);
}

void operateWholeSaller(ChefsInfo *chfsInf, int fptr)
{

    char ingredient;
    char actualName[INGREDIENT_NAME_MAX_SIZE];
    char actualName2[INGREDIENT_NAME_MAX_SIZE];
    int count;

    while (count != 0)
    {
        chfsInf->isCooked = false;

        count = read(fptr, &ingredient, sizeof(char)); 
        chfsInf->delivered[0] = ingredient;
        
        count = read(fptr, &ingredient, sizeof(char));
        chfsInf->delivered[1] = ingredient;
    
        extendIngredientName(chfsInf->delivered[0], actualName);
        extendIngredientName(chfsInf->delivered[1], actualName2);
        fprintf(stderr, "the wholesaler delivers %s and %s\n", actualName, actualName2);

        // new  line
        count = read(fptr, &ingredient, sizeof(char));

        fprintf(stderr, "the wholesaler is waiting for the dessert\n");
        
        while (chfsInf->isCooked == false)
        {
        }
        
        fprintf(stderr, "the wholesaler has obtained the dessert and left to sell it\n");
    }

    chfsInf->isOver = true;
}

void extendIngredientName(char ingredient, char actualName[INGREDIENT_NAME_MAX_SIZE])
{
    switch (ingredient)
    {
    case 'M':
        sprintf(actualName, "%s", "milk");
        break;

    case 'F':
        sprintf(actualName, "%s", "flour");
        break;

    case 'W':
        sprintf(actualName, "%s", "walnuts");
        break;

    case 'S':
        sprintf(actualName, "%s", "sugar");
        break;
    }

}

bool isDeliveredPrecisely(ChefsInfo chfsInf, int chefID)
{

    if (chfsInf.delivered[0] == chfsInf.chef[chefID].lakeIng[0])
    {
        if (chfsInf.delivered[1] == chfsInf.chef[chefID].lakeIng[1])
        {
            return true;
        }
    }
    else if (chfsInf.delivered[1] == chfsInf.chef[chefID].lakeIng[0])
    {
        if (chfsInf.delivered[0] == chfsInf.chef[chefID].lakeIng[1])
        {
            return true;
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
    chfsInf->isOver = false;
    setChefsAbsents(chfsInf);

    if (sem_init(&(chfsInf->mutex), 1, 1) < 0)
    {
        perror("semaphore initilization");
        exit(0);
    }

    chfsInf->delivered = (char *)malloc(INGREDIENT_NUM * sizeof(char));

    for (int i = 0; i < CHEF_NUM; i++)
    {
        pthread_create(&(chfsInf->chef[i].tID), NULL, &chefs, chfsInf);
    }
}
void destructChefs(ChefsInfo *chfsInf)
{

    free(chfsInf->delivered);

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

void checkUsage(int argc, char *argv[], char **filePath)
{
    int opt;

    if (argc != 3)
    {
        fprintf(stderr, "Ooooooppppppsssss! You entered wrong arguments. Usage: \n");
        fprintf(stderr, "./program -i filePath\n");
        exit(EXIT_FAILURE);
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
            exit(EXIT_FAILURE);
            break;
        }
    }
}