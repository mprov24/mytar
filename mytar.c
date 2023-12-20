#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "mytar.h"
#include "tools.h"
#include "tarCreate.h"
#include "tarList.h"
#include "tarExtract.h"

static char usage[256];
static char missingMode[256];
static char invalidModesMsg[256]; 

void handleArgs(int argc, char *argv[], enum mainMode_t *mainMode, 
                int *vOpt, int *sOpt, int *fOpt)
{
    sprintf(usage, "usage: %s [ctxvS[f tarfile]] [file1 [ file2 [...] ] ]\n",
            argv[0]);
    
    sprintf(missingMode, "%s: you must specify at least one of the "
                         "'ctx' options.\n", argv[0]);

    sprintf(invalidModesMsg, "%s: you must specify only one of the "
                             "'ctx' options\n", argv[0]);
    if (argc == 1)
    {
        fprintf(stderr, missingMode);
        fprintf(stderr, usage);
        exit(1);
    }

    getModes(argv[1], mainMode, vOpt, sOpt, fOpt);

    if (*mainMode == NA)
    {
        fprintf(stderr, "%s%s", missingMode, usage);
        exit(EXIT_FAILURE);
    }
    if (!*fOpt)
    {
        fprintf(stderr, "f option is required\n%s", usage);
        exit(EXIT_FAILURE);
    }
    if (*fOpt && argc < 3)
    {
        fprintf(stderr, usage);
        exit(EXIT_FAILURE);
    }
}

void getModes(char *modes, enum mainMode_t *mainMode, 
              int *vOpt, int *sOpt, int *fOpt)
{
    int i = 0;
    char c;
    while((c=modes[i]) != '\0')
    {
        switch(c) 
        {
            case 'c':
                if (*mainMode != NA)
                {
                    fprintf(stderr, "%s%s", invalidModesMsg, usage);
                    exit(EXIT_FAILURE);
                }
                *mainMode = create;
                break;
            case 't':
                if (*mainMode != NA)
                {
                    fprintf(stderr, "%s%s", invalidModesMsg, usage);
                    exit(EXIT_FAILURE);
                }
                *mainMode = listing;
                break;
            case 'x':
                if (*mainMode != NA)
                {
                    fprintf(stderr, "%s%s", invalidModesMsg, usage);
                    exit(EXIT_FAILURE);
                }
                *mainMode = extract;
                break;
            case 'v': *vOpt = 1;
                break;
            case 'S': *sOpt = 1;
                break;
            case 'f': *fOpt = 1;
                break;
            default:
                fprintf(stderr, "%c: Unrecognized mode\n%s", c, usage);
                exit(EXIT_FAILURE);
        }
        i++;
    }
}

int isInArgList(char *args[], int numArgs, char* fullName)
{
    // DETERMINE IF TAR ENTRY IS IN USER INPUT PATHS //

    // extract/list all if no args provided
    if (numArgs == 0)
        return 1;

    int i = 0;
    while (i < numArgs)
    {
        // first compare tar name to user input name(s)
        if (strncmp(fullName, args[i], strlen(args[i])) == 0)
        {
            // we need to ensure user input name
            // doesn't terminate in the middle of a directory name in tarfile
            if (args[i][strlen(args[i])-1] != '/')
            {
                if (fullName[strlen(args[i])] == '/' || 
                    fullName[strlen(args[i])] == '\0')
                    return 1;
            }
            else
                return 1;
        }
        i++;
    }
    return 0;
}

uint64_t computeChecksum(headerData_t headerData)
{
    int i;
    unsigned char c;
    uint64_t chksum = 0;
    for (i=0; i<BLOCK_SIZE; i++)
    {
        c = headerData.bin[i];
        if (i >= CHKSUM_OFFSET && i<(CHKSUM_OFFSET+CHKSUM_SIZE))
            c = ' ';
        chksum += c;
    }
    return chksum;
}

int verifyHeader(headerData_t headerData, int strict)
{
    // check for CHKSUM, MAGIC NUMBER, and VERSION
    if (strncmp(headerData.fields.magic, "ustar", 5) != 0)
        return 0;
    uint64_t headerChkSum = strtoul(headerData.fields.chksum, NULL, 8);
    if (headerChkSum != computeChecksum(headerData))
        return 0;
    if (strict)
    {
        if (headerData.fields.version[0] != '0' || 
            headerData.fields.version[1] != '0' ||
            strcmp(headerData.fields.magic, "ustar") != 0)
            return 0;
    }
    return 1;
}

void combineName(headerData_t headerData, char *fullName)
{
    // COMBINE PREFIX AND NAME //

    // declare names, in case names are not null terminated, do it ourselves
    char name[NAME_SIZE+1];
    char prefix[PREFIX_SIZE+1];
    name[NAME_SIZE] = '\0';
    prefix[PREFIX_SIZE] = '\0';

    // create full name from name and prefix
    strncpy(prefix, headerData.fields.prefix, PREFIX_SIZE);
    strncpy(name, headerData.fields.name, NAME_SIZE);
    int prefixLen = strlen(prefix);
    if (prefixLen != 0)
    {
        strcpy(fullName, prefix);
        fullName[prefixLen] = '/';
        prefixLen++;
    }
    strcpy(fullName+prefixLen, name);
}

void loopArchives(int argc, char *argv[], int vOpt, int sOpt, 
                  enum mainMode_t mainMode)
{
    // READ TAR AND LOOP THROUGH ENTRIES, FOR LISTING AND EXTRACT MODES //

    FILE *fin = fopen(argv[2], "r");
    if (!fin)
        bailPerror(argv[2]);

    char fullName[NAME_SIZE+PREFIX_SIZE+2];

    long headerIndex = 0;   // byte index at start of current block
    uint64_t recordSize;
    headerData_t headerData;
 
    // loop until we hit an empty block, signalling end
    // (or if we just started and headerIndex is 0)
    while (!headerIndex || computeChecksum(headerData) != (' '*CHKSUM_SIZE))
    {
        // read first block into headerData struct
        fseek(fin, headerIndex, SEEK_SET);
        if (fread(headerData.bin, sizeof(char), BLOCK_SIZE, fin) != BLOCK_SIZE)
            bail("unexpected EOF\n");
        headerIndex += BLOCK_SIZE; 

        recordSize = strtoul(headerData.fields.size, NULL, 8);

        // create full name from name and prefix
        combineName(headerData, fullName);

        // check header integrity
        if (!verifyHeader(headerData, sOpt))
        {
            // if not valid because data is null, exit loop
            if (computeChecksum(headerData) != (' '*CHKSUM_SIZE))
                bail("malformed header\n");
        }
        // if valid and path was specified by user
        else if (isInArgList(argv+3, argc-3, fullName))
        {
            if (mainMode == extract)
                extractFile(fullName, headerData, recordSize, 
                            fin, headerIndex);
            // else if listing in verbose mode
            else if (vOpt)
                listVerbose(headerData, recordSize);
            // print file in listing mode, or extracting in verbose mode
            if (mainMode == listing || vOpt)
                printf("%s\n", fullName);
        }
        // update header index based on how much data block archive contained
        headerIndex += (recordSize / BLOCK_SIZE) * BLOCK_SIZE + 
                       (recordSize % BLOCK_SIZE > 0) * BLOCK_SIZE;
    }

    // read block following the first empty block hit
    fseek(fin, headerIndex, SEEK_SET);
    // if block ends too early, or is non empty, bail
    if (fread(headerData.bin, sizeof(char), BLOCK_SIZE, fin) != BLOCK_SIZE)
        bail("unexpected EOF\n");
    if (computeChecksum(headerData) != (' '*CHKSUM_SIZE))
        bail("encountered empty block followed by nonempty block\n");

    fclose(fin);
}

int main(int argc, char *argv[])
{
    enum mainMode_t mainMode = NA;
    int vOpt, sOpt, fOpt;
    vOpt = sOpt = fOpt = 0;
    // get arguments
    handleArgs(argc, argv, &mainMode, &vOpt, &sOpt, &fOpt);
    // execute main mode
    if (mainMode == listing || mainMode == extract)
        loopArchives(argc, argv, vOpt, sOpt, mainMode);
    else if (mainMode == create)
        executeCreate(argc, argv, vOpt, sOpt);
    return 0;
}


