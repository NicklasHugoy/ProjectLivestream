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
#include <assert.h>

#define MAX_UNIQUE_USERS 50
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
    int *whitelistScore;
    int scoreThreshold;
    char username[20];
    int chatDelay;
};
struct OneWord
{
    int wordlength;
    char storedWord[];
};

void UserInputDialog(int *scoreThreshold, char streamerUsername[]);
int ConvertTimestamp(char timestamp[]);
void ReadChatLog(struct Line *line, FILE* inputFile, int *hasReachedEndOfFile);
int SingleChatterDelay(struct Users user[], int chatDelay, struct Line newMessage);
void OutputToFile(struct Line line, FILE *outputFile, struct Line savedMessages[], struct Config configFile, struct Users user[]);
void SaveMessage(struct Line line, struct Line savedMessages[]);
int CompareWithLastMessages(struct Line line, struct Line savedMessages[]);
int ContainsProblematicCharacter(char *stringToCheck);
struct Config GetConfig(char filePath[]);
int ContainsWhiteListedWords(struct Line line, struct Config config);
int OnlyNumber(char *input);
int ContainsWord(struct Line line, char *word);
int MentionsStreamer(struct Line line, char *username);
int CalculatePoints(struct Line line, struct Config configFile);
int MessageSpamDetection(struct Line message, int filter);
int WordCompare(struct OneWord words[], int totalWords, int sizeOfSingleWords);
int SortWords(const void *a, const void *b);

int main(void)
{
    FILE *outputFile = fopen("TextFiles/Output.txt", "w");
    FILE *inputFile = fopen("TextFiles/ForsenLoL_ChatLog_29-10.txt", "r");
    struct Line line;
    struct Line savedMessages[NUMBER_OF_SAVED_MESSAGES];
    int hasReachedEndOfFile = 0;
    struct Users user[MAX_UNIQUE_USERS];
    int spamDetected=0;

    struct Config configFile = GetConfig("TextFiles/config.txt");

    printf("Processing...\n");
    while(hasReachedEndOfFile != 1)
    {
        ReadChatLog(&line, inputFile, &hasReachedEndOfFile);
        if(hasReachedEndOfFile != 1)
        {
            if(MessageSpamDetection(line, 2))
            {
                spamDetected++;
                continue;
            }
            OutputToFile(line, outputFile, savedMessages, configFile, user);

        }
    }
    printf("%d message was seen as spam\n", spamDetected );
    fclose(outputFile);
    return 0;
}

/* Reads config file and returns Config struct with the settings */
struct Config GetConfig(char filePath[])
{
    FILE *configFile = fopen(filePath, "r");
    struct Config configStruct;
    char line[1024];

    if(configFile != NULL)
    {
        char *information;
        int i = 0, amountOfWords;
        int bytesNow;
        int bytesConsumed;

        /* reads one line at a time */
        while(fgets(line, sizeof(line), configFile) != NULL)
        {
            /* Pointer to part of string after '=' */
            information = strchr(line, '=')+1;

            /* First line in config file */
            switch(i)
            {
                case 0: /*  Amount of whitelisted words */
                  sscanf(information, " %d", &amountOfWords);
                  configStruct.amountOfWords = amountOfWords;
                  break;
                case 1: /* Whitelisted words */
                  bytesConsumed = 0;
                  configStruct.words = malloc(amountOfWords * sizeof(char*));
                  for(int j = 0; j < amountOfWords; j++)
                  {
                      configStruct.words[j] = malloc(10);
                      /* Returns amount of bytes consumed to be able to continue from where it stopped */
                      sscanf(information+bytesConsumed, " %s%n", configStruct.words[j], &bytesNow);
                      bytesConsumed += bytesNow;
                  }
                  break;
                case 2:
                  sscanf(information+bytesConsumed, " %d", &configStruct.mentionsScore);
                  break;
                case 3:
                  bytesConsumed = 0;
                  configStruct.whitelistScore = malloc(amountOfWords*sizeof(int));
                  for(int j = 0; j < amountOfWords; j++)
                  {
                     sscanf(information+bytesConsumed, " %d%n", &configStruct.whitelistScore[j], &bytesNow);
                     bytesConsumed += bytesNow;
                  }
                  break;
                case 4:
                  sscanf(information, " %d", &configStruct.scoreThreshold);
                  break;
                case 5:
                  sscanf(information, " %s", configStruct.username);
                  break;
                case 6:
                  sscanf(information, " %d", &configStruct.chatDelay);
                  break;
            }
            i++;
        }
        fclose(configFile);
    }
    else
    {
        configFile = fopen(filePath, "w");

        printf("Config file doesn't exist. Please enter values for config file\n");

        /* Default values if config file dosn't exist */
        printf("Number of whitelisted words: "); scanf(" %s", line);
        fprintf(configFile, "Number of whitelisted words        = %s\n", line);
        printf("Whitelisted words: "); scanf(" %[ -~]", line);
        fprintf(configFile, "Whitelisted words                  = %s\n", line);
        printf("Score for mentions : "); scanf(" %s", line);
        fprintf(configFile, "Score for mentions                 = %s\n", line);
        printf("Score for each whitelisted words: "); scanf(" %[0-9 ]", line);
        fprintf(configFile, "Score for each whitelisted words   = %s\n", line);
        printf("Score required: "); scanf(" %s", line);
        fprintf(configFile, "Score required                     = %s\n", line);
        printf("Streamer username: "); scanf(" %s", line);
        fprintf(configFile, "Streamer username                  = %s\n", line);
        printf("Chat Delay in seconds: "); scanf(" %s", line);
        fprintf(configFile, "Chat Delay in seconds              = %s", line);

        fclose(configFile);
        return GetConfig(filePath);
    }

    return configStruct;
}

/*  Read 1 line in the chatlog and
    returns 1 if there aren't more lines to read */
void ReadChatLog(struct Line *line, FILE* inputFile, int *hasReachedEndOfFile)
{
    char message[2001];
    if(inputFile != NULL)
    {
        /* Check if fscanf successfully has assigned values to 2 variables */
        if(fscanf(inputFile, " [%[0-9 -:] UTC] %[0-9A-z_]: %[^\n]",
            line->timeStamp, line->username, message) == 3)
        {
            *hasReachedEndOfFile = 0;
            if(ContainsProblematicCharacter(message))
            {
                strcpy(line->message,"ERROR - Problematic characters - ERROR");
            }
            else
            {
                strcpy(line->message, message);
            }
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

void OutputToFile(struct Line line, FILE *outputFile, struct Line savedMessages[], struct Config configFile, struct Users user[])
{
    if(CalculatePoints(line, configFile) >= configFile.scoreThreshold)
    {
        if(SingleChatterDelay(user, configFile.chatDelay, line) > configFile.chatDelay)
        {
            if(CompareWithLastMessages(line, savedMessages))
                return;

            SaveMessage(line, savedMessages);
            fprintf(outputFile,"[%s] %s: %s\n", line.timeStamp, line.username, line.message);
        }
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
    		if(i == 0)
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
            return 1;
        }
    }
    return 0;
}

/*Checks for ascii chars*/
int ContainsProblematicCharacter(char *stringToCheck)
{
    int messageLenght = strlen(stringToCheck);
    char normalText[] = "abcdefghijklmnopqrstuvwxyz,!:;=+- ?.ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    int falseChars = 1;
    for(int i=0; i<messageLenght; i++)
    {
        if(strchr(normalText, stringToCheck[i]) == NULL)
        {
            falseChars++;
        }
    }
    if((messageLenght / falseChars) <= 2)
    {
        return 1;
    }
    return 0;
}

/*  Reuturns 1 if the message contains one of the whitelisted words,
    specified in the config file */
int ContainsWhiteListedWords(struct Line line, struct Config config)
{
    int score = 0;
    for(int i = 0; i < config.amountOfWords; i++)
    {
        /* If message contains the word return 1 */
        if(ContainsWord(line, config.words[i]))
            score += config.whitelistScore[i];
    }
    return score;
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

    points += ContainsWhiteListedWords(line, configFile);

    if(MentionsStreamer(line, configFile.username))
        points += configFile.mentionsScore;

    return points;
}
/*modtager en besked og et filter.
deler beskeden op i substrings som består at enkelte ord.
finder ud af om beskeden sees som spam eller ej.
giver en boolsk værdi tilbage (0 eller 1)*/
int MessageSpamDetection(struct Line message, int filter)
{
    int i, j=0;
    int messageTotalLength;
    int messageOffset=0;
    int totalWords=0;
    int singleWordlength=0;
    int longestWord=0;
    int messageIssue=0;
    int sizeOfSingleWords;
    struct OneWord *SingleWords;

    messageTotalLength=strlen(message.message);
    /*her læses beskeden igennem og finder ud af hvor mange ord beskeden har*/
    for(i=0; i<messageTotalLength; i++)
    {
        singleWordlength++;
        if(message.message[i]==' ' || i==messageTotalLength-1)
        {
            totalWords++;
            if(longestWord<singleWordlength)
            {
                longestWord=singleWordlength;
            }
            singleWordlength=0;
        }
    }
    /*her alloceres plads til den bedsked som blev læst igennem tidligere*/
    sizeOfSingleWords = sizeof(struct OneWord) + ((10+longestWord)*sizeof(char));
    SingleWords = malloc(totalWords * sizeOfSingleWords);
    if(SingleWords == NULL)
    {
        printf("Error allocating with Malloc for SingeWords\n");
        assert(SingleWords == NULL);
    }
    /*her gøres det samme som før, men når den finder det ord
    vil den ligge det over i allayet som blev lavet lige før*/
    for(i=0; i<messageTotalLength; i++)
    {
        singleWordlength++;
        if(message.message[i]==' ' || i==messageTotalLength-1)
        {
            /*den første if ser efter om den er kommet til det sidste ord i beskeden
            den sortere også mellemrum fra når de er lige efter hinanden*/
            if(i==messageTotalLength-1 || singleWordlength == 1)
            {   /*når den finder et mellemrum vil den ligge længeden af ordet in i wordlength
                og den vil ligge ordet inde i char arrayet.*/
                if(i==messageTotalLength-1)
                    {
                        SingleWords[j].wordlength = singleWordlength;
                        strncpy(SingleWords[j].storedWord, message.message+messageOffset, singleWordlength);
                        memset(SingleWords[j].storedWord+singleWordlength,'\0',1);
                    }
                messageOffset = i;
                singleWordlength=0;
            }
            else
            {
                SingleWords[j].wordlength = singleWordlength;
                strncpy(SingleWords[j].storedWord, message.message+messageOffset, singleWordlength);
                memset(SingleWords[j].storedWord+singleWordlength,'\0',1);
                messageOffset = i+1;
                j++;
                singleWordlength=0;
            }

        }
    }
    /*bruger en anden function til at finde dupliceret ord*/
    messageIssue = WordCompare(SingleWords, totalWords, sizeOfSingleWords);
    /*her finder den ud af om beskedens indhold kan sees som spam
    ud fra den filter brugeren har givet progammet*/
    if (messageIssue!=-1)
    {
        if(messageIssue>filter)
            return 1;
        else
            return 0;
    }
    else
        return 0;

free (SingleWords);
return messageIssue;
}
/*finder hvor mange dupliceret ord der er og antal af unikke.
giver antallet af gange dupliceret ord går op i unikke*/
int WordCompare(struct OneWord words[], int totalWords, int sizeOfSingleWords)
{
    int i, duplicatedWords=0, uniqueWords=0, issues;
    /*sortere ordne så alle de dupliceret ord kommer efter hinanden*/
    qsort(words, totalWords, sizeOfSingleWords, SortWords);
    /*efter sorteringen ser den efter om ordet er det samme som det næste*/
    for(i=0; i<totalWords; i++)
    {
        if(i!=totalWords-1)
        {
            if (strcmp(words[i].storedWord, words[i+1].storedWord)==0)
            {
                duplicatedWords++;
            }
            else
                uniqueWords++;
        }
        else
        {
            if (strcmp(words[i].storedWord, words[i-1].storedWord)!=0)
            {
               uniqueWords++;
            }
        }
    }
    /*her findes en problemstørrelse ud fra hvor mange unikke ord der er og hvor mange dupliceret ord*/
    if (duplicatedWords!=0)
    {
        issues=uniqueWords / duplicatedWords;
    }
    else
        issues=-1;

    return issues;
}
/*sortere ord i OneWord struct arrayet*/
int SortWords(const void *a, const void *b)
{
    struct OneWord *OneWord1 = (struct OneWord*) a;
    struct OneWord *OneWord2 = (struct OneWord*) a;

    if(strcmp(OneWord1->storedWord, OneWord2->storedWord)==1)
        return 1;
    else if (strcmp(OneWord1->storedWord, OneWord2->storedWord)==-1)
        return -1;
    else
        return 0;
}
