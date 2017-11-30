/********************************/
/* Analyse af livestream data   */
/*          DAT - A412          */
/*                              */
/*         20/12 - 2017         */
/********************************/

#include <stdio.h>
#include <stdlib.h>

#define MAX_LINES 500

struct User
{
    char username[40];
    char timeStamp[40];
    char message[100];
};

void ReadChatLog(struct User users[], FILE *inputFile);
int CountMessages(FILE *inputFile);

int main(void)
{
    FILE *inputFile = fopen("TextFiles/Cryaotic_ChatLog_21-11.txt", "r");
    FILE *outputFile = fopen("TextFiles/Output.txt", "w");
    struct User *users;
    int numberOfMessages;

    numberOfMessages = CountMessages(inputFile);

    users = (struct User *)malloc(numberOfMessages * sizeof(struct User));

    ReadChatLog(users, inputFile);

    fclose(outputFile);
    return 0;
}

void ReadChatLog(struct User users[], FILE *inputFile)
{
    int i=0;

    while(fscanf(inputFile, " [%[0-9 -:] UTC] %[0-9A-z_]: %[^\n]",
        users[i].timeStamp, users[i].username, users[i].message) == 3)
    {
        printf("timeStamp: %s\n", users[i].timeStamp);
        printf("Username: %s\n", users[i].username);
        printf("Message: %s\n\n\n",users[i].message);
        i++;
    }

    printf("i: %d\n", i);
    printf("test: %s\n\n\n",users[293].message);

    fclose(inputFile);
}

int CountMessages(FILE *inputFile)
{
    int amountOfMessages = 0, scanres;
    char buffer[MAX_LINES];


    if(inputFile != NULL)
    {
        scanres = fscanf(inputFile, " [%[^\n]", buffer);
        while(scanres == 1)
        {
            amountOfMessages++;
            scanres = fscanf(inputFile, " [%[^\n]", buffer);
        }
    }
    else
    {
        printf("Problem with file, exiting program...\n");
        exit(EXIT_FAILURE);
    }
    rewind(inputFile);
    return amountOfMessages;
}
