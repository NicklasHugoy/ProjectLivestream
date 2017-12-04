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
#define MAX_UNIC_USERS 15
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
	char timeStamp[40];
};

void UserInputDialog(int *scoreThreshold, char streamerUsername[]);
int ConvertTimestamp(char timestamp[]);
void ReadChatLog(struct Message *message, FILE* inputFile);
int CountAmountOfLines(char path[]);
int SingleChatterDelay(struct Users user[], int chatDelay, struct Message newMessage);
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
    int chatDelay=10;
    char streamerUsername[30];
    struct Users user[MAX_UNIC_USERS];

    UserInputDialog(&scoreThreshold, streamerUsername);

    int numberOfMessages = CountAmountOfLines("TextFiles/Cryaotic_ChatLog_21-11.txt");

    for(int i=0; i < numberOfMessages; i++)
    {
        ReadChatLog(&message, inputFile);

        if(SingleChatterDelay(user, chatDelay, message)<chatDelay)
        	continue;
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
	int hours, minutes, seconds, tempresult, results;

	sscanf(timestamp,"%s %d:%d:%d", tempTime, &hours, &minutes, &seconds);

	tempresult= hours*MIN + minutes;
	results= tempresult*SEC + seconds;
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

/*Checker om den nye bruger har skrevet før og om han må skrive igen.*/
int SingleChatterDelay(struct Users user[], int chatDelay, struct Message newMessage){
	int i;
	int userindex = MAX_UNIC_USERS;
	int result=chatDelay+1;
	struct Users newUser;

	strcpy(newUser.username, newMessage.username);
	strcpy(newUser.timeStamp, newMessage.timeStamp);

	for(i=0; i<MAX_UNIC_USERS; i++)
	{
		if(strcmp(newUser.username, user[i].username)==0)
		{
			result = ConvertTimestamp(newUser.timeStamp) - ConvertTimestamp(user[i].timeStamp);
			userindex=i+1;
			break;
		}
	}
	for(i=userindex-1; i>0; i--)
	{
		user[i]=user[i-1];
		if(i==1)
		{
			user[i-1]=newUser;
		}
	}
	return result;
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
