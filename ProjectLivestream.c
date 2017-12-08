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

#define MAX_UNIQUE_USERS 15
#define NUMBER_OF_SAVED_MESSAGES 5

struct Line
{
    char username[40];
    char timeStamp[40];
    char message[501];
    int score;
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
    int mentionsScore;
    int whitelistScore;
    int scoreThreshold;
    char username[20];
    int chatDelay;
};

void UserInputDialog(int *scoreThreshold, char streamerUsername[]);
int ConvertTimestamp(char timestamp[]);
void ReadChatLog(struct Line *line, FILE* inputFile, int *hasReachedEndOfFile);
int SingleChatterDelay(struct Users user[], int chatDelay, struct Line newMessage);
void OutputToFile(struct Line line, FILE *outputFile, struct Line savedMessages[], int chatDelay, struct Users user[]);
void SaveMessage(struct Line line, struct Line savedMessages[]);
int CompareWithLastMessages(struct Line line, struct Line savedMessages[]);
int CheckMessage(FILE *inputFile);
struct Config GetConfig(char filePath[]);
int ContainsWhiteListedWords(struct Line line, struct Config config);
int OnlyNumber(char *input);
int ContainsWord(struct Line line, char *word);
int MentionsStreamer(struct Line line, char *username);
int CalculatePoints(struct Line line, struct Config configFile);

int main(void)
{
    FILE *outputFile = fopen("TextFiles/Output.txt", "w");
    FILE *inputFile = fopen("TextFiles/ForsenLoL_ChatLog_29-10.txt", "r");
    struct Line line;
    struct Line savedMessages[NUMBER_OF_SAVED_MESSAGES];
    int hasReachedEndOfFile = 0;
    struct Users user[MAX_UNIQUE_USERS];

    struct Config configFile = GetConfig("TextFiles/config.txt");

    while(hasReachedEndOfFile != 1)
    {
        ReadChatLog(&line, inputFile, &hasReachedEndOfFile);
        if(hasReachedEndOfFile != 1)
        {
            if(CompareWithLastMessages(line, savedMessages)==0)
                line.score -= 2;
            if(CalculatePoints(line, configFile) >= configFile.scoreThreshold)
                OutputToFile(line, outputFile, savedMessages, configFile.chatDelay, user);
        }
    }
    printf("%d\n", sizeof(char*));
    fclose(outputFile);
    return 0;
}

/* Reads config file and returns Config struct with the settings */
struct Config GetConfig(char filePath[])
{
    FILE *configFile = fopen(filePath, "r");
    struct Config configStruct;

    if(configFile != NULL)
    {
        char line[1024], *information;
        int i = 0, amountOfWords;

        /* reads one line at a time */
        while(fgets(line, sizeof(line), configFile) != NULL)
        {
            /* Pointer to part of string after '=' */
            information = strchr(line, '=')+1;

            /* First line in config file */
            if(i==0) /*  Amount of whitelisted words */
            {
                sscanf(information, " %d", &amountOfWords);
                configStruct.amountOfWords = amountOfWords;
            }
            else if(i==1) /* Whitelisted words */
            {
                int bytesNow;
                int bytesConsumed = 0;

                configStruct.words = malloc((amountOfWords * sizeof(char*)));
                for(int j = 0; j < amountOfWords; j++)
                {
                    configStruct.words[j] = malloc(10);
                    /*  %n Returns amount of bytes consumed to be able to
                        continue from were it stoped, next time the loop runs*/
                    sscanf(information+bytesConsumed, " %s%n", configStruct.words[j], &bytesNow);
                    bytesConsumed += bytesNow;
                }
            }
            else if(i==2)
            {
                sscanf(information, " %d", &configStruct.mentionsScore);
            }
            else if(i==3)
            {
                sscanf(information, " %d", &configStruct.whitelistScore);
            }
            else if(i==4)
            {
                sscanf(information, " %d", &configStruct.scoreThreshold);
            }
            else if(i==5)
            {
                sscanf(information, " %s", configStruct.username);
            }
            else if(i==6)
            {
                sscanf(information, " %d", &configStruct.chatDelay);
            }
            i++;
        }
    }
    else
    {
        /* Default values if config file dosn't exist */
        configStruct.amountOfWords = 0;
        configStruct.mentionsScore = 1;
        configStruct.scoreThreshold = 0;
        configStruct.username[0] = '\0';
        configStruct.chatDelay = 10;

    }
    fclose(configFile);
    return configStruct;
}

/*  Read 1 line in the chatlog and
    returns 1 if there aren't more lines to read */
void ReadChatLog(struct Line *line, FILE* inputFile, int *hasReachedEndOfFile)
{
    if(inputFile != NULL)
    {
        /* Check if fscanf successfully has assigned values to 2 variables */
        if(fscanf(inputFile, " [%[0-9 -:] UTC] %[0-9A-z_]: ",
            line->timeStamp, line->username) == 2)
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

    /* Check to make sure message dosn't contain problemmatic characters */
    if(CheckMessage(inputFile))
    {
        fscanf(inputFile, "%[^\n]", line->message);
    }
    else
    {   /* Replace message with error */
        strcpy(line->message,"ERROR - Problematic characters - ERROR");
    }
}

/* convert timestamp to int of seconds */
int ConvertTimestamp(char timestamp[])
{
	const int MIN = 60;
	const int SEC = 60;
	char tempTime[12];
	int hours, minutes, seconds, tempresult, results;

	sscanf(timestamp,"%s %d:%d:%d", tempTime, &hours, &minutes, &seconds);

	tempresult = hours * MIN + minutes;
	results = tempresult * SEC + seconds;
	return results;
}

void OutputToFile(struct Line line, FILE *outputFile, struct Line savedMessages[], int chatDelay, struct Users user[])
{
    if(SingleChatterDelay(user, chatDelay, line)>chatDelay)
    {
        SaveMessage(line, savedMessages);
        fprintf(outputFile,"[%s] %s: %s\n", line.timeStamp, line.username, line.message);
    }
}

/*Checker om den nye bruger har skrevet før og om han må skrive igen.*/
int SingleChatterDelay(struct Users user[], int chatDelay, struct Line newMessage)
{
	int i;
	int userindex = MAX_UNIQUE_USERS-1;
	int result = chatDelay+1;
	struct Users newUser;

	strcpy(newUser.username, newMessage.username);
	strcpy(newUser.timeStamp, newMessage.timeStamp);

	for(i = 0; i < MAX_UNIQUE_USERS; i++)
	{
		if(strcmp(newUser.username, user[i].username) == 0)
		{
			result = ConvertTimestamp(newUser.timeStamp) - ConvertTimestamp(user[i].timeStamp);
			userindex = i;
			break;
		}
	}
    if(result > chatDelay)
    {
        for(i = userindex; i >= 0; i--)
    	{
    		if(i==0)
    		{
    			user[i] = newUser;
    		}
            else
            {
                user[i] = user[i-1];
            }
    	}
    }
	return result;
}

/* Save the message to the start of and array and shift the array*/
void SaveMessage(struct Line line, struct Line savedMessages[])
{
    /* Shift elements in array */
    for(int k = NUMBER_OF_SAVED_MESSAGES-1; k > 0; k--)
    {
        savedMessages[k] = savedMessages[k-1];
    }
    /* Save new message */
    savedMessages[0] = line;
}

/*  Compare the message with an array of older messages
    and checks if it's the same message */
int CompareWithLastMessages(struct Line line, struct Line savedMessages[])
{
    for(int i = 0; i < NUMBER_OF_SAVED_MESSAGES; i++)
    {
        if(strcmp(line.message, savedMessages[i].message) == 0)
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
    char normalText[] = "abcdefghijklmnopqrstuvwxyz,!:;=+- ?.ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    int falseChars=0;
    while(currentChar != '\n')
    {
        currentChar = fgetc(inputFile);

        if(strchr(normalText, currentChar) == NULL)
            {
                falseChars++;
            }
        if(currentChar == EOF)
            break;
    }
    messageEnd = ftell(inputFile);
    messageLenght = messageEnd - messageStart;
    if((messageLenght / falseChars) <= 2)
    {
        return 0;
    }
    fseek(inputFile, messageStart, SEEK_SET);
    return 1;

}

/*  Reuturns 1 if the message contains one of the whitelisted words,
    specified in the config file */
int ContainsWhiteListedWords(struct Line line, struct Config config)
{
    for(int i = 0; i < config.amountOfWords; i++)
    {
        /* If message contains the word return 1 */
        if(ContainsWord(line, config.words[i]))
            return 1;
    }
    return 0;
}

/* Return 1 if the @username is in the message */
int MentionsStreamer(struct Line line, char *username)
{
    /* New arrary with size 1 byte larger than username */
    char mention[strlen(username)+1];

    /* Adds a @ character in front of the username */
    mention[0] = '@';
    strcpy(mention+1, username);

    return ContainsWord(line, mention);
}

int ContainsWord(struct Line line, char *word)
{
    if(strstr(line.message, word))
        return 1;
    return 0;
}

int CalculatePoints(struct Line line, struct Config configFile)
{
    int points = 0;
    if(ContainsWhiteListedWords(line, configFile))
        points += configFile.whitelistScore;
    if(MentionsStreamer(line, configFile.username))
        points += configFile.mentionsScore;
    return points;
}
