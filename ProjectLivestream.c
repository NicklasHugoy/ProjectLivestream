/********************************/
/* Analyse af livestream data   */
/*          DAT - A412          */
/*                              */
/*         20/12 - 2017         */
/********************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINES 500

struct User
{
    char username[40];
    char timeStamp[40];
    char message[500];
};

void UserInputDialog(int *scoreThreshold, char streamerUsername[]);
int ConvertTimestamp(char timestamp[]);
void ReadChatLog(struct User *user, FILE* inputFile);
int CountAmountOfLines(char path[]);
void OutputToFile(struct User user, FILE *outputFile);

int main(void)
{
    FILE *outputFile = fopen("TextFiles/Output.txt", "w");
    FILE *inputFile = fopen("TextFiles/Cryaotic_ChatLog_21-11.txt", "r");
    struct User user;
    int scoreThreshold;
    char streamerUsername[30];

    UserInputDialog(&scoreThreshold, streamerUsername);

    int numberOfMessages = CountAmountOfLines("TextFiles/Cryaotic_ChatLog_21-11.txt");

    for(int i=0; i < numberOfMessages; i++)
    {
        ReadChatLog(&user, inputFile);
        printf("timeStamp: %s\n", user.timeStamp);
        printf("Username: %s\n", user.username);
        printf("Message: %s\n\n\n",user.message);
    }

    fclose(outputFile);
    return 0;
}

void UserInputDialog(int *scoreThreshold, char streamerUsername[])
{
    printf("Please enter score threshold: ");
    scanf("%d", scoreThreshold);
    printf("Please enter your username: ");
    scanf("%s", streamerUsername);
}

void ReadChatLog(struct User *user, FILE* inputFile)
{
    if(inputFile != NULL)
    {
        fscanf(inputFile, " [%[0-9 -:] UTC] %[0-9A-z_]: %[^\n]",
            user->timeStamp, user->username, user->message);
    }
    else
    {
        printf("Problem with file, exiting program...\n");
        exit(EXIT_FAILURE);
    }
}

/*convert timestamp to int of seconds*/
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

void OutputToFile(struct User user, FILE *outputFile)
{
	fprintf(outputFile,"%s %s %s", user.timeStamp, user.username, user.message);
}