/********************************
 * Analyse af livestream data   *
 *          DAT - A412          *
 *                              *
 *         20/12 - 2017         *
 ********************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_UNIC_USERS 15
#define NUMBER_OF_SAVED_MESSAGES 5

struct Message
{
    char username[40];
    char timeStamp[40];
    char message[501];
    int points;
};

struct Users
{
	char username[40];
	char timeStamp[40];
};

struct Config
{
    int amountOfWords;
    char **words;
};

void UserInputDialog(int *scoreThreshold, char streamerUsername[]);
int ConvertTimestamp(char timestamp[]);
void ReadChatLog(struct Message *message, FILE* inputFile, int *hasReachedEndOfFile);
int SingleChatterDelay(struct Users user[], int chatDelay, struct Message newMessage);
void OutputToFile(struct Message message, FILE *outputFile, struct Message savedMessages[], int chatDelay, struct Users user[]);
void SaveMessage(struct Message message, struct Message savedMessages[]);
int CompareWithLastMessages(struct Message message, struct Message savedMessages[]);
int CheckMessage(FILE *inputFile);
struct Config GetConfig(char filePath[]);
int ContainsWhiteListedWords(struct Message message, struct Config config);
int OnlyNumber(char *input);
int ContainsWord(struct Message message, char *word);
int MentionsStreamer(struct Message message, char *username);

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

    struct Config configFile = GetConfig("TextFiles/config.txt");

    while(hasReachedEndOfFile != 1)
    {
        ReadChatLog(&message, inputFile, &hasReachedEndOfFile);
        if(hasReachedEndOfFile != 1)
        {
            if(CompareWithLastMessages(message, savedMessages)==0)
                continue;
            OutputToFile(message, outputFile, savedMessages, chatDelay, user);
        }
    }
    fclose(outputFile);

    printf("amount: %d\n", configFile.amountOfWords);
    printf("text: %s\n", configFile.words[0]);

    return 0;
}

/* Reads config file and returns Config struct with the settings */
struct Config GetConfig(char filePath[])
{
    FILE *configFile = fopen(filePath, "r");
    struct Config configStruct;

    if(configFile != NULL)
    {
        char line[1024];
        int i=0, amountOfWords;

        /* reads one line at a time */
        while(fgets(line, sizeof(line), configFile) != NULL)
        {
            /* First line in config file */
            if(i==0) /*  Amount of whitelisted words */
            {
                sscanf(line, " Number_of_whitelisted_words = %d", &amountOfWords);
                configStruct.amountOfWords = amountOfWords;
            }
            else if(i==1) /* Whitelisted words */
            {
                int bytesNow;
                int bytesConsumed=0;
                char *test;

                /* Pointer to part of string after '=' */
                test = strchr(line, '=')+1;

                configStruct.words = malloc(amountOfWords * sizeof(char*));
                for(int j=0; j<amountOfWords; j++)
                {
                    configStruct.words[j] = malloc(10);
                    /* Returns amount of bytes consumed to be able to continue from were it stoped */
                    sscanf(test+bytesConsumed, " %s%n", configStruct.words[j], &bytesNow);
                    bytesConsumed += bytesNow;
                }
            }
            i++;
        }
    }
    fclose(configFile);
    return configStruct;
}

void UserInputDialog(int *scoreThreshold, char streamerUsername[])
{
    char tempChar[10];

    /* Prompter for input til point grænse, input tager kun imod tal */
    do
    {
      printf("Enter number for point limit: ");
      scanf(" %s", tempChar);
    }
    while(OnlyNumber(tempChar) != 1);

    *scoreThreshold = atoi(tempChar);

    printf("Please enter your username: ");
    scanf("%s", streamerUsername);
}

/* Funktion, som tjekker efter bogstaver i input */
int OnlyNumber(char *input)
{
  for(int i = 0; input[i] != '\0'; i++)
  {
    if(isdigit(input[i]) == 0)
      return 0;
  }
  return 1;
}

void ReadChatLog(struct Message *message, FILE* inputFile, int *hasReachedEndOfFile)
{
    if(inputFile != NULL)
    {
        if(fscanf(inputFile, " [%[0-9 -:] UTC] %[0-9A-z_]: ",
            message->timeStamp, message->username)==2)
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
    if(CheckMessage(inputFile))
    {
        fscanf(inputFile, "%[^\n]", message->message);
    }
    else
    {
        strcpy(message->message,"ERROR - Problematic characters - ERROR");
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

void OutputToFile(struct Message message, FILE *outputFile, struct Message savedMessages[], int chatDelay, struct Users user[])
{
    if(SingleChatterDelay(user, chatDelay, message)>chatDelay)
    {
        SaveMessage(message, savedMessages);
        fprintf(outputFile,"[%s] %s: %s\n", message.timeStamp, message.username, message.message);
    }
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

/*Checks for ascii chars*/
int CheckMessage(FILE *inputFile)
{
    int messageStart = ftell(inputFile);
    int messageEnd;
    int messageLenght;
    int currentChar;
    char normalText[]="abcdefghijklmnopqrstuvwxyz,!:;=+- ?.ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    int falseChars=0;
    while(currentChar != '\n')
    {
        currentChar = fgetc(inputFile);

        if(strchr(normalText, currentChar)==NULL)
            {
                falseChars++;
            }
        if(currentChar==EOF)
            break;
    }
    messageEnd = ftell(inputFile);
    messageLenght = messageEnd - messageStart;
    if((messageLenght/falseChars)<=2)
    {
        return 0;
    }
    fseek(inputFile, messageStart, SEEK_SET);
    return 1;

}

int ContainsWhiteListedWords(struct Message message, struct Config config)
{
    for(int i=0; i<config.amountOfWords; i++)
    {
        /* If message contains the word return 1 */
        if(ContainsWord(message, config.words[i]))
            return 1;
    }
    return 0;
}

int MentionsStreamer(struct Message message, char *username)
{
    char mention[strlen(username)+1];

    mention[0] = '@';
    strcpy(mention+1, username);

    return ContainsWord(message, mention);
}

int ContainsWord(struct Message message, char *word)
{
    if(strstr(message.message, word))
        return 1;
    return 0;
}

int CalculatePoints(struct Message message, struct Config configFile)
{
    return 0;
}
