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

struct Line
{
    char username[40];
    char timeStamp[40];
    char message[531];
};

struct User
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
    int timeScore;
    char chatlogPath[100];
    char outputPath[100];
    int amountOfSavedMessages;
};
struct OneWord
{
    int wordlength;
    char* storedWord;
};

struct Config GetConfig(char filePath[]);
void ConfigDialog(struct Config configFile, char filePath[]);
int ReadChatLog(struct Line *line, FILE* inputFile, int *hasReachedEndOfFile, FILE* problematicChars);
int ContainsProblematicCharacter(char *stringToCheck);
int MessageSpamDetection(struct Line message, int filter);
int WordCompare(struct OneWord words[], int totalWords);
void OutputToFile(struct Line line, FILE *outputFile, struct Line savedMessages[], struct Config configFile, struct User users[]);
int CalculatePoints(struct Line line, struct Config configFile, struct User users[]);
int ContainsWhiteListedWords(struct Line line, struct Config config);
int ContainsWord(struct Line line, char *word);
char *stringToLowerCase(char *string);
char *wordFind(char *str, char *word);
int MentionsStreamer(struct Line line, char *username);
int SingleChatterDelay(struct User users[], int chatDelay, struct Line newMessage);
int ConvertTimestamp(char timestamp[]);
int CompareWithLastMessages(struct Line line, struct Line savedMessages[], struct Config configFile);
void SaveMessage(struct Line line, struct Line savedMessages[], struct Config configFile);

int main(void)
{
    FILE *inputFile, *outputFile;
    FILE *spamFile, *problematicChars;
    struct Line line;
    int hasReachedEndOfFile = 0;
    struct User users[MAX_UNIQUE_USERS];
    int spamDetected=0;
    int spamissuecount=0;
    int problematiskeBeskeder=0;

    struct Config configFile = GetConfig("config.txt");
    ConfigDialog(configFile, "TextFiles/config.txt");

    struct Line savedMessages[configFile.amountOfSavedMessages];

    inputFile = fopen(configFile.chatlogPath, "r");
    outputFile = fopen(configFile.outputPath, "w");

    spamFile = fopen("spamfile.txt", "w");
    problematicChars = fopen("problematicChars.txt","w");
    printf("Processing...\n");
    while(hasReachedEndOfFile != 1)
    {
        if(ReadChatLog(&line, inputFile, &hasReachedEndOfFile, problematicChars))
        {
            if(hasReachedEndOfFile != 1)
            {   
                spamissuecount = MessageSpamDetection(line, 2);
                if(spamissuecount>=0 && spamissuecount<=3)
                {
                    fprintf(spamFile,"%d [%s] %s: %s\n", spamissuecount, line.timeStamp, line.username, line.message);
                    spamDetected++;
                    continue;
                }
                if(CompareWithLastMessages(line, savedMessages, configFile))
                    continue;
                OutputToFile(line, outputFile, savedMessages, configFile, users);
            }
        }
        else
            problematiskeBeskeder++;
    }
    printf("%d messages was seens as problematic\n",problematiskeBeskeder);
    printf("%d messages was seen as spam\n", spamDetected );
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
                  sscanf(information, " %d", &configStruct.mentionsScore);
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
                case 7:
                  sscanf(information, " %d", &configStruct.timeScore);
                  break;
                case 8:
                  sscanf(information, " %s", configStruct.chatlogPath);
                  break;
                case 9:
                  sscanf(information, " %s", configStruct.outputPath);
                  break;
                case 10:
                  sscanf(information, " %d", &configStruct.amountOfSavedMessages);
                  break;

            }
            i++;
        }
        fclose(configFile);
    }
    else
    {
        configFile = fopen(filePath, "w");

        printf("Please enter values for config file\n");

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
        fprintf(configFile, "Chat Delay in seconds              = %s\n", line);
        printf("Score for each second between a users messages: "); scanf(" %s", line);
        fprintf(configFile, "Score for each second between a users messages              = %s\n", line);
        printf("Path to chatlog : "); scanf(" %s", line);
        fprintf(configFile, "Path to chatlog               = %s\n", line);
        printf("Output Path: "); scanf(" %s", line);
        fprintf(configFile, "Output Path                = %s\n", line);
        printf("Amount of previous messages to check against: "); scanf(" %s", line);
        fprintf(configFile, "Amount of previous messages to check against      = %s\n", line);

        fclose(configFile);
        return GetConfig(filePath);
    }

    return configStruct;
}

void ConfigDialog(struct Config configFile, char filePath[])
{
    char userInput;
    printf("Current configuration file: \n\n");

    printf("Number of whitelisted words:\t\t\t %d\n", configFile.amountOfWords);
    for(int i=0; i<configFile.amountOfWords; i++)
    {
        printf("%d points if the message contains the word:\t %s\n", configFile.whitelistScore[i], configFile.words[i]);
    }
    printf("Score for mentions:\t\t\t\t %d\n", configFile.mentionsScore);
    printf("Score required:\t\t\t\t\t %d\n", configFile.scoreThreshold);
    printf("Streamer username:\t\t\t\t %s\n", configFile.username);
    printf("Chat Delay in seconds:\t\t\t\t %d\n", configFile.chatDelay);
    printf("Score for each second between a users messages:\t %d\n", configFile.timeScore);
    printf("Path to chatlog:\t\t\t\t\t %s\n", configFile.chatlogPath);
    printf("Output Path:\t\t\t\t %s\n", configFile.outputPath);
    printf("Amount of previous messages to check against:\t\t\t %d\n", configFile.amountOfSavedMessages);

    printf("\nDo you want to create a new config file? (Y/N)\n");
    scanf(" %c", &userInput);
    printf("\n\n");
    if(userInput == 'Y' || userInput == 'y')
    {
        remove(filePath);
        configFile = GetConfig(filePath);
    }
}

/*  Read 1 line in the chatlog.
    hasReachedEndOfFile is set to 1 if there aren't more lines to read.
    Function returns 0 if the current message should be skipped */
int ReadChatLog(struct Line *line, FILE* inputFile, int *hasReachedEndOfFile, FILE* problematicChars)
{
    char message[2001];
    int charIssues=0;
    if(inputFile != NULL)
    {
        /* Check if fscanf successfully has assigned values to 2 variables */
        if(fscanf(inputFile, " [%[0-9 -:] UTC] %[0-9A-z_]: %[^\n]",
            line->timeStamp, line->username, message) == 3)
        {
            *hasReachedEndOfFile = 0;
            charIssues = ContainsProblematicCharacter(message); 
            if(charIssues>20)
            {
                fprintf(problematicChars, "%d [%s] %s: %s\n", charIssues, line->timeStamp, line->username, message);
                return 0;
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
    return 1;
}

/*Checks for ascii chars*/
int ContainsProblematicCharacter(char *stringToCheck)
{
    int messageLenght = strlen(stringToCheck);
    char normalText[] = "abcdefghijklmnopqrstuvwxyz<>@^\"/\\_´'&#()[]{}$*,!:;=+- ?.ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    int falseChars = 0;
    for(int i=0; i<messageLenght; i++)
    {
        if(strchr(normalText, stringToCheck[i]) == NULL)
        {
            falseChars++;
        }
    }
    return falseChars;
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
    int messageIssue=-1;
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
    SingleWords = malloc(totalWords*sizeof(struct OneWord));
    if(SingleWords == NULL)
    {
        printf("Error allocating with Malloc for SingeWords\n");
        assert(SingleWords == NULL);
    }
    for(i=0; i<totalWords; i++)
    {
        SingleWords[i].storedWord = malloc((longestWord+1)*sizeof(char));
    }
    for (i=0; i<totalWords; i++)
    {
        if (SingleWords[i].storedWord == NULL)
        {
            printf("Error in allocation with malloc for storedword %d\n",i);
            assert(SingleWords[i].storedWord == NULL);
        }
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
                messageOffset = i+1;
                singleWordlength=0;
            }
            else
            {
                SingleWords[j].wordlength = singleWordlength-1;
                strncpy(SingleWords[j].storedWord, message.message+messageOffset, singleWordlength-1);
                memset(SingleWords[j].storedWord+singleWordlength-1,'\0',1);
                messageOffset = i+1;
                j++;
                singleWordlength=0;
            }

        }
    }
    /*bruger en anden function til at finde dupliceret ord*/
    if(totalWords>1)
        messageIssue = WordCompare(SingleWords, totalWords);
    

    for(i=0; i<totalWords; i++)
    {
        free (SingleWords[i].storedWord);
    }
    free (SingleWords);
    return messageIssue;
}

/*finder hvor mange dupliceret ord der er og antal af unikke.
giver antallet af gange dupliceret ord går op i unikke*/
int WordCompare(struct OneWord words[], int totalWords)
{
    int i, duplicatedWords=0, uniqueWords, issues, j;
    /*tager et ord og sammenligner med de næste ord til at den rammer en duplikation.*/
    for(i=0; i<totalWords; i++)
    {
        j=i;
        while(j<totalWords)
        {
            if(j!=totalWords-1)
            {
                if (strcmp(words[i].storedWord, words[j+1].storedWord)==0)
                {
                    duplicatedWords++;
                    break;
                }
            }
            j++;
        }
    }
    /*her findes en problemstørrelse ud fra hvor mange unikke ord der er og hvor mange dupliceret ord*/
    uniqueWords = totalWords - duplicatedWords;
    if (duplicatedWords!=0)
    {
        issues = uniqueWords / duplicatedWords;
    }
    else
        issues=-1;

    return issues;
}

void OutputToFile(struct Line line, FILE *outputFile, struct Line savedMessages[], struct Config configFile, struct User users[])
{
    int score = CalculatePoints(line, configFile, users);
    if(score >= configFile.scoreThreshold)
    {
        SaveMessage(line, savedMessages, configFile);
        fprintf(outputFile,"[%s] %s: %s\n", line.timeStamp, line.username, line.message);
    }
}

int CalculatePoints(struct Line line, struct Config configFile, struct User users[])
{
    int points = 0;

    points += ContainsWhiteListedWords(line, configFile);

    if(MentionsStreamer(line, configFile.username))
        points += configFile.mentionsScore;

    points += (configFile.timeScore * (SingleChatterDelay(users, configFile.chatDelay, line)/5));
    return points;
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

int ContainsWord(struct Line line, char *word)
{
    char *wordLCase = stringToLowerCase(word);
    char *messageLCase = stringToLowerCase(line.message);

    char *containWord = wordFind(messageLCase, wordLCase);

    if(containWord)
    {
        free(wordLCase);
        free(messageLCase);
        return 1;
    }
    free(wordLCase);
    free(messageLCase);
    return 0;
}

char *stringToLowerCase(char *string)
{
    int stringLength = strlen(string)+1;
    char *lowerCaseString = malloc(sizeof(char)*stringLength);

    for(int i=0; i<stringLength; i++)
    {
        lowerCaseString[i] = tolower(string[i]);
    }
    return lowerCaseString;
}

char *wordFind(char *str, char *word)
{
    char *p = NULL;
    int len = strlen(word);

    if (len > 0)
    {
        for (p = str; (p = strstr(p, word)) != NULL; p++)
        {
            if (p == str || !isalnum(p[-1]))
            {
                if (!isalnum(p[len]))
                    break;  /* we have a match! */
                p += len;   /* next match is at least len+1 bytes away */
            }
        }
    }
    return p;
}

/* Return 1 if the @username is in the message */
int MentionsStreamer(struct Line line, char *username)
{
    return ContainsWord(line, username);
}

/*Checker om den nye bruger har skrevet før og om han må skrive igen.*/
int SingleChatterDelay(struct User users[], int chatDelay, struct Line newMessage)
{
	int i;
	int userindex = MAX_UNIQUE_USERS-1;
	int result = chatDelay+1;
	struct User newUser;

	strcpy(newUser.username, newMessage.username);
	strcpy(newUser.timeStamp, newMessage.timeStamp);

	for(i = 0; i < MAX_UNIQUE_USERS; i++)
	{
		if(strcmp(newUser.username, users[i].username) == 0)
		{
			result = ConvertTimestamp(newUser.timeStamp) - ConvertTimestamp(users[i].timeStamp);
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
    			users[i] = newUser;
    		}
            else
            {
                users[i] = users[i-1];
            }
    	}
    }
	return result;
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

/*  Compare the message with an array of older messages
    and checks if it's the same message */
int CompareWithLastMessages(struct Line line, struct Line savedMessages[], struct Config configFile)
{
    for(int i = 0; i < configFile.amountOfSavedMessages; i++)
    {
        if(strcmp(line.message, savedMessages[i].message) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/* Save the message to the start of and array and shift the array*/
void SaveMessage(struct Line line, struct Line savedMessages[], struct Config configFile)
{
    /* Shift elements in array */
    for(int k = configFile.amountOfSavedMessages - 1; k > 0; k--)
    {
        savedMessages[k] = savedMessages[k-1];
    }
    /* Save new message */
    savedMessages[0] = line;
}
