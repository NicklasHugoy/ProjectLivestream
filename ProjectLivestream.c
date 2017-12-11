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

#define MAX_UNIQUE_USERS 15
#define NUMBER_OF_SAVED_MESSAGES 5

struct Message
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
struct OneWord
{
    int wordlength;
    char storedWord[];
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
int CalculatePoints(struct Message message, struct Config configFile);
int MessageSpamDetection(struct Message message, int filter);
int WordCompare(struct OneWord words[], int totalWords, int sizeOfSingleWords);
int SortWords(const void *a, const void *b);


int main(void)
{
    FILE *outputFile = fopen("TextFiles/Output.txt", "w");
    FILE *inputFile = fopen("TextFiles/ForsenLoL_ChatLog_29-10.txt", "r");
    struct Message message;
    struct Message savedMessages[NUMBER_OF_SAVED_MESSAGES];
    int hasReachedEndOfFile;
    struct Users user[MAX_UNIQUE_USERS];
    int spamDetected=0;

    struct Config configFile = GetConfig("TextFiles/config.txt");

    while(hasReachedEndOfFile != 1)
    {
        ReadChatLog(&message, inputFile, &hasReachedEndOfFile);
        if(hasReachedEndOfFile != 1)
        {
            if(MessageSpamDetection(message, 2))
            {
                spamDetected++;
                continue;
            }
            if(CompareWithLastMessages(message, savedMessages)==0)
                message.score -= 2;
            if(CalculatePoints(message, configFile) >= configFile.scoreThreshold)
                OutputToFile(message, outputFile, savedMessages, configFile.chatDelay, user);
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

    if(configFile != NULL)
    {
        char line[1024], *information;
        int i=0, amountOfWords;

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
                int bytesConsumed=0;

                configStruct.words = malloc(amountOfWords * sizeof(char*));
                for(int j=0; j<amountOfWords; j++)
                {
                    configStruct.words[j] = malloc(10);
                    /* Returns amount of bytes consumed to be able to continue from were it stoped */
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
    fclose(configFile);
    return configStruct;
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
        strcpy(message->message,"ERROR-Problematic-characters-ERROR");
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
	int userindex = MAX_UNIQUE_USERS-1;
	int result=chatDelay+1;
	struct Users newUser;

	strcpy(newUser.username, newMessage.username);
	strcpy(newUser.timeStamp, newMessage.timeStamp);

	for(i=0; i<MAX_UNIQUE_USERS; i++)
	{
		if(strcmp(newUser.username, user[i].username)==0)
		{
			result = ConvertTimestamp(newUser.timeStamp) - ConvertTimestamp(user[i].timeStamp);
			userindex=i;
			break;
		}
	}
    if(result > chatDelay)
    {
        for(i=userindex; i>=0; i--)
    	{
    		if(i==0)
    		{
    			user[i]=newUser;
    		}
            else
            {
                user[i]=user[i-1];
            }
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
    int points=0;
    if(ContainsWhiteListedWords(message, configFile))
        points+=configFile.whitelistScore;
    if(MentionsStreamer(message, configFile.username))
        points+=configFile.mentionsScore;
    return points;
}
/*modtager en besked og et filter.
deler beskeden op i substrings som består at enkelte ord.
finder ud af om beskeden sees som spam eller ej.
giver en boolsk værdi tilbage (0 eller 1)*/
int MessageSpamDetection(struct Message message, int filter)
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