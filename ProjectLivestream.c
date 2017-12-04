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
    char message[500];
};

void ReadChatLog(struct User user, char path[]);
int CountAmountOfLines(char path[]);

int main(void)
{
    FILE *outputFile = fopen("TextFiles/Output.txt", "w");
    struct User user;

    ReadChatLog(user, "TextFiles/Cryaotic_ChatLog_21-11.txt");

    fclose(outputFile);
    return 0;
}

void ReadChatLog(struct User user, char path[])
{
    FILE *inputFile = fopen(path, "r");

    while(fscanf(inputFile, " [%[0-9 -:] UTC] %[0-9A-z_]: %[^\n]",
        user.timeStamp, user.username, user.message) == 3)
    {
        printf("timeStamp: %s\n", user.timeStamp);
        printf("Username: %s\n", user.username);
        printf("Message: %s\n\n\n",user.message);
    }
    fclose(inputFile);
}

/* Returns the amount of lines in the inputFIle */
int CountAmountOfLines(char path[])
{
    FILE *inputFile = fopen(path, "r");
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
    fclose(inputFile);
    return amountOfMessages;
}
