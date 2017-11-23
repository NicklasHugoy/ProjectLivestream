#include <stdio.h>

int main()
{
    FILE *inputFile = fopen("TextFiles/Cryaotic_ChatLog_21-11.txt", "r");
    FILE *outputFile = fopen("TextFiles/Output.txt", "w");


    fclose(inputFile);
    fclose(outputFile);

    return 0;
}
