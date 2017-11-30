/********************************/
/* Analyse af livestream data   */
/*          DAT - A412          */
/*                              */
/*         20/12 - 2017         */
/********************************/

#include <stdio.h>

struct User
{
    char username[30];
    char timeStamp[30];
    char message[60];
};

void ReadChatLog(FILE* inputFile, struct User users[]);

int main(void)
{
    FILE *inputFile = fopen("TextFiles/Cryaotic_ChatLog_21-11.txt", "r");
    FILE *outputFile = fopen("TextFiles/Output.txt", "w");
    struct User users[30];

    ReadChatLog(inputFile, users);

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
