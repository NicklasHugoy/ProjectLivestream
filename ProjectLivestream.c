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
#define NUMBER_OF_SAVED_MESSAGES 5

struct Message
{
    char username[40];
    char timeStamp[40];
    char message[500];
    int points;
};

struct Users
{
	char username[40];
	int timestamp;
};

void UserInputDialog(int *scoreThreshold, char streamerUsername[]);
int ConvertTimestamp(char timestamp[]);
void ReadChatLog(struct Message *message, FILE* inputFile);
int CountAmountOfLines(char path[]);
void OutputToFile(struct Message message, FILE *outputFile, struct Message savedMessages[]);
void SaveMessage(struct Message message, struct Message savedMessages[]);
int CompareWithLastMessages(struct Message message, struct Message savedMessages[]);

int main(void)
{
    FILE *outputFile = fopen("TextFiles/Output.txt", "w");
    FILE *inputFile = fopen("TextFiles/Cryaotic_ChatLog_21-11.txt", "r");
    struct Message message;
    struct Message savedMessages[NUMBER_OF_SAVED_MESSAGES];
    int scoreThreshold;
    char streamerUsername[30];

    UserInputDialog(&scoreThreshold, streamerUsername);

    int numberOfMessages = CountAmountOfLines("TextFiles/Cryaotic_ChatLog_21-11.txt");

    for(int i=0; i < numberOfMessages; i++)
    {
        ReadChatLog(&message, inputFile);
        if(i >= NUMBER_OF_SAVED_MESSAGES)
        {
            if(CompareWithLastMessages(message, savedMessages)==0)
                continue;
        }
        OutputToFile(message, outputFile, savedMessages);
        printf("%s\n", message.message);
    }

    for(int i=0; i<NUMBER_OF_SAVED_MESSAGES; i++)
    {
        printf("%s\n", savedMessages[i].message);
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

void ReadChatLog(struct Message *message, FILE* inputFile)
{
    if(inputFile != NULL)
    {
        fscanf(inputFile, " [%[0-9 -:] UTC] %[0-9A-z_]: %[^\n]",
            message->timeStamp, message->username, message->message);
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

void OutputToFile(struct Message message, FILE *outputFile, struct Message savedMessages[])
{
    SaveMessage(message, savedMessages);
	fprintf(outputFile,"[%s]%s : %s\n", message.timeStamp, message.username, message.message);
}

void SaveMessage(struct Message message, struct Message savedMessages[])
{
    /* Shift elements in array */
    for(int k = NUMBER_OF_SAVED_MESSAGES-1; k > 0; k--)
    {
        savedMessages[k]=savedMessages[k-1];
    }
    /* Save new message */
    savedMessages[0]=message;
}

int CompareWithLastMessages(struct Message message, struct Message savedMessages[])
{
    for(int i=0; i<NUMBER_OF_SAVED_MESSAGES; i++)
    {
        if(strcmp(message.message, savedMessages[i].message) == 0)
        {
            return 0;
        }
    }
    return 1;
}
