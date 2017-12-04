/********************************/
/* Analyse af livestream data   */
/*          DAT - A412          */
/*                              */
/*         20/12 - 2017         */
/********************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct User
{
    char username[30];
    char timeStamp[30];
    char message[60];
};

void ReadChatLog(FILE* inputFile, struct User users[]);
int ConvertTimestamp(char timestamp[]);

int main(void)
{
    FILE *inputFile = fopen("TextFiles/Cryaotic_ChatLog_21-11.txt", "r");
    FILE *outputFile = fopen("TextFiles/Output.txt", "w");
    struct User users[30];

    ReadChatLog(inputFile, users);

    for (int i = 0; i < 10; ++i)
    {
    	ConvertTimestamp(users[i].timeStamp);
    }

    fclose(outputFile);
    return 0;
}

void ReadChatLog(FILE* inputFile, struct User users[])
{
    for(int i=0; i<10; i++)
    {
        fscanf(inputFile, " [%[0-9 -:] UTC] %[0-9A-z_]: %[ -~]\n", users[i].timeStamp, users[i].username, users[i].message);

        printf("timeStamp: %s\n", users[i].timeStamp);
        printf("Username: %s\n", users[i].username);
        printf("Message: %s\n\n\n",users[i].message);
    }

    fclose(inputFile);
}


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