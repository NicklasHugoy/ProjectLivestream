/********************************/
/* Analyse af livestream data   */
/*          DAT - A412          */
/*                              */
/*         20/12 - 2017         */
/********************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_UNIC_USERS 15
#define NUMBER_OF_SAVED_MESSAGES 5

struct Message
{
    char username[40];
    char timeStamp[40];
    char message[2001];
    int points;
};

struct Users
{
	char username[40];
	char timeStamp[40];
};

void UserInputDialog(int *scoreThreshold, char streamerUsername[]);
int ConvertTimestamp(char timestamp[]);
void ReadChatLog(struct Message *message, FILE* inputFile, int *hasReachedEndOfFile);
int SingleChatterDelay(struct Users user[], int chatDelay, struct Message newMessage);
void OutputToFile(struct Message message, FILE *outputFile, struct Message savedMessages[]);
void SaveMessage(struct Message message, struct Message savedMessages[]);
int CompareWithLastMessages(struct Message message, struct Message savedMessages[]);

int main(void)
{
    FILE *outputFile = fopen("TextFiles/Output.txt", "w");
    FILE *inputFile = fopen("TextFiles/ForsenLoL_ChatLog_29-10.txt", "r");
    struct Message message;
    struct Message savedMessages[NUMBER_OF_SAVED_MESSAGES];
    int scoreThreshold, chatDelay=10, hasReachedEndOfFile;
    char streamerUsername[30];
    struct Users user[MAX_UNIC_USERS];

    UserInputDialog(&scoreThreshold, streamerUsername);

    while(hasReachedEndOfFile != 1)
    {
        ReadChatLog(&message, inputFile, &hasReachedEndOfFile);
        if(hasReachedEndOfFile != 1)
        {
            if(SingleChatterDelay(user, chatDelay, message)<chatDelay)
                continue;
            if(CompareWithLastMessages(message, savedMessages)==0)
                continue;
            OutputToFile(message, outputFile, savedMessages);
        }
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

void ReadChatLog(struct Message *message, FILE* inputFile, int *hasReachedEndOfFile)
{
    if(inputFile != NULL)
    {
        if(fscanf(inputFile, " [%[0-9 -:] UTC] %[0-9A-z_]: %[^\n]",
            message->timeStamp, message->username, message->message)==3)
        {
            *hasReachedEndOfFile = 0;
        }
        else
        {
            *hasReachedEndOfFile = 1;
        }
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

void OutputToFile(struct Message message, FILE *outputFile, struct Message savedMessages[])
{
    SaveMessage(message, savedMessages);
	fprintf(outputFile,"[%s] %s: %s\n", message.timeStamp, message.username, message.message);
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
