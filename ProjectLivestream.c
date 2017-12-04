/********************************/
/* Analyse af livestream data   */
/*          DAT - A412          */
/*                              */
/*         20/12 - 2017         */
/********************************/

#include <stdio.h>
<<<<<<< HEAD
#include <string.h>
#include <stdlib.h>
=======
#include <stdlib.h>

#define MAX_LINES 500
>>>>>>> d96fba3bf7c68342ce41dc964d9f53858a1125ce

struct User
{
    char username[40];
    char timeStamp[40];
    char message[500];
};

<<<<<<< HEAD
void ReadChatLog(FILE* inputFile, struct User users[]);
int ConvertTimestamp(char timestamp[]);
=======
void ReadChatLog(struct User user, char path[]);
int CountAmountOfLines(char path[]);
>>>>>>> d96fba3bf7c68342ce41dc964d9f53858a1125ce

int main(void)
{
    FILE *outputFile = fopen("TextFiles/Output.txt", "w");
    struct User user;

    ReadChatLog(user, "TextFiles/Cryaotic_ChatLog_21-11.txt");

    for (int i = 0; i < 10; ++i)
    {
    	ConvertTimestamp(users[i].timeStamp);
    }

    fclose(outputFile);
    return 0;
}

void ReadChatLog(struct User user, char path[])
{
    FILE *inputFile = fopen(path, "r");

    if(inputFile != NULL)
    {
        while(fscanf(inputFile, " [%[0-9 -:] UTC] %[0-9A-z_]: %[^\n]",
            user.timeStamp, user.username, user.message) == 3)
        {
            printf("timeStamp: %s\n", user.timeStamp);
            printf("Username: %s\n", user.username);
            printf("Message: %s\n\n\n",user.message);
        }
    }
    else
    {
        printf("Problem with file, exiting program...\n");
        exit(EXIT_FAILURE);
    }

    fclose(inputFile);
}

<<<<<<< HEAD

int ConvertTimestamp(char timestamp[])
{
	const int MIN = 60;
	const int SEC = 60;
	char tempTime[12];
	int hours, minutes, seconds, results;

	sscanf(timestamp,"%s %d:%d:%d", tempTime, &hours, &minutes, &seconds);

	results= hours*MIN + minutes;
	results+= results*SEC + seconds;
	printf("%d\n",results);
	return results;

}
=======
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
>>>>>>> d96fba3bf7c68342ce41dc964d9f53858a1125ce
