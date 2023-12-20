#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "tarList.h"

void getTimeStr(headerData_t headerData, char *timeStr)
{
    time_t mtime;

    struct tm *timeStruct;
    mtime = strtoul(headerData.fields.mtime, NULL, 8);
    timeStruct = localtime(&mtime);
    strftime(timeStr, 17, "%Y-%m-%d %H:%M", timeStruct);
}

char getTypeFlagChar(headerData_t *headerData)
{
    char typeflag;
    switch (*(headerData->fields.typeflag))
    {
        case '2': typeflag = 'l'; break;
        case '5': typeflag = 'd'; break;
        default:  typeflag = '-'; break;
    }
    return typeflag;
}
void listVerbose(headerData_t headerData, long recordSize)
{

    // get info from tar header
    mode_t mode = strtol(headerData.fields.mode, NULL, 8); 
    char typeflag = getTypeFlagChar(&headerData);
    char timeStr[17];
    getTimeStr(headerData, timeStr);

    char UGName[UNAME_SIZE+GNAME_SIZE+1]; // uname/gname
    strcpy(UGName, headerData.fields.uname);
    UGName[strlen(UGName)+1] = '\0';
    UGName[strlen(UGName)] = '/';
    strcpy(UGName+strlen(UGName), headerData.fields.gname);

    putchar(typeflag);

    // read permissions from mode
    int i;
    char perm;
    for (i=8;i>=0;i--)
    {
        switch (i%3)
        {
            case 0: perm = 'x'; break;
            case 1: perm = 'w'; break;
            case 2: perm = 'r'; break;
        }
        if (mode & (1 << i))
            putchar(perm);
        else
            putchar('-');
    }

    printf(" %-17s", UGName);
    printf(" %8ld", recordSize);
    printf(" %s ", timeStr);
}
