#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <arpa/inet.h>

#include "tarCreate.h"

void writeData(FILE *fout, unsigned long *blockIndex, char *pathInput)
{
    // WRITE DATA SECTION OF TARFILE FOR CREATION //
    FILE *fin = fopen(pathInput, "r");

    char buff[BLOCK_SIZE];
    int bytesRead;
    fseek(fout, *blockIndex, SEEK_SET);

    // write 512 size blocks
    while ((bytesRead = fread(buff, sizeof(char), BLOCK_SIZE, fin)))
    {
        memset(buff, 0, BLOCK_SIZE);
        fwrite(buff, sizeof(char), BLOCK_SIZE, fout);
        (*blockIndex) += BLOCK_SIZE;
    }
    fclose(fin); 
}
void executeCreate(int argc, char *argv[], int vOpt, int sOpt)
{
    FILE *fout = fopen(argv[2], "w");
    if (!fout)
        bailPerror(argv[2]);
    unsigned long blockIndex = 0;
    char *pathInput;
    int i;

    // loop through input file arguments
    for (i = 3; i < argc; i++)
    {
        pathInput = argv[i];
        // archive each path recursively
        archivePath(fout, &blockIndex, pathInput, vOpt, sOpt);
    }

    // write final empty blocks
    fseek(fout, blockIndex, SEEK_SET);
    char emptybuff[BLOCK_SIZE*2];
    memset(emptybuff, 0, BLOCK_SIZE*2);
    fwrite(emptybuff, 1, BLOCK_SIZE*2, fout);

    fclose(fout);
}

void recurseDirDFS(FILE *fout, unsigned long *blockIndex, char *pathInput, 
                    int vOpt, int sOpt)
{
    // RECURSE DIRECTORIES IN DFS FOR TAR CREATION //
    
    struct stat statInfo;
    int pathLen = strlen(pathInput);
    DIR *dp;
    if (!(dp = opendir(pathInput)))
        fprintf(stderr, "cannot open dir: %s\n", pathInput);

    struct dirent *entinfo;
    // traverse only directories in path directory
    while ((entinfo=readdir(dp)))
    {
        strcat(pathInput, entinfo->d_name);
        if (lstat(pathInput, &statInfo))
            fprintf(stderr, "cannot stat %s...\n", pathInput);
        else if (S_ISDIR(statInfo.st_mode) && 
                 strcmp(entinfo->d_name, ".") && 
                 strcmp(entinfo->d_name, ".."))
            archivePath(fout, blockIndex, pathInput, vOpt, sOpt);
        pathInput[pathLen] = '\0';
    }
    // rewind directory to read non directories
    rewinddir(dp);
    // read and archive only regular files and links
    while ((entinfo=readdir(dp)))
    {
        strcat(pathInput, entinfo->d_name);
        if (lstat(pathInput, &statInfo))
            fprintf(stderr, "cannot stat %s...\n", pathInput);
        else if (S_ISREG(statInfo.st_mode) || S_ISLNK(statInfo.st_mode))
            archivePath(fout, blockIndex, pathInput, vOpt, sOpt);
        pathInput[pathLen] = '\0';
    }
    closedir(dp);
}

void archivePath(FILE *fout, unsigned long *blockIndex, char *pathInput, 
                 int vOpt, int sOpt)
{
    // ARCHIVE PATH FOR TAR CREATION //

    struct stat statInfo;
    int pathLen = strlen(pathInput);

    // stat to get type of file
    if (lstat(pathInput, &statInfo))
    {
        fprintf(stderr, "cannot stat %s...\n", pathInput);
        return;
    }
    
    // if user did not put a slash at end of directory, insert one
    if (S_ISDIR(statInfo.st_mode) && pathInput[pathLen-1] != '/')
    {
        pathInput[pathLen+1] = '\0';
        pathInput[pathLen] = '/';
    }

    // print for verbose mode
    if (vOpt)
        fprintf(stderr, "%s\n", pathInput);
    
    // exit if there was an error writing header
    if (!writeHeader(fout, blockIndex, pathInput, statInfo, sOpt))
        return;

    // write data and exit if this is a reg file
    if (S_ISREG(statInfo.st_mode))
        writeData(fout, blockIndex, pathInput);

    // continue down directory, doing a DFS if this is a directory
    else if (S_ISDIR(statInfo.st_mode))
    {
        recurseDirDFS(fout, blockIndex, pathInput, vOpt, sOpt);
    }
}

int writeHeader(FILE *fout, unsigned long *blockIndex, char *pathInput, 
                 struct stat statInfo, int sOpt)
{
    // initialize headerData to all 0
    headerData_t headerData;
    memset(headerData.bin, 0, BLOCK_SIZE);

    // skip file if uid too long in strict mode
    if (!writeGUID(statInfo, &headerData, sOpt))
    {
        fprintf(stderr, "%s: Unable to create conforming header. Skipping.\n",
                pathInput);
        return 0;
    }

    // skip file if name does not fit in header
    if (!splitName(pathInput, &headerData))
    {
        fprintf(stderr, "%s: path does not fit in header, skipping\n", 
                pathInput);
        return 0;
    }

    // filetype dependent info 
    if (S_ISREG(statInfo.st_mode))
    {
        sprintf(headerData.fields.size, "%011o", statInfo.st_size);
        headerData.fields.typeflag[0] = '0';
    }
    else if (S_ISLNK(statInfo.st_mode))
    {
        sprintf(headerData.fields.size, "%011o", 0);
        headerData.fields.typeflag[0] = '2';
        char linkVal[LINKNAME_SIZE+1];
        memset(linkVal, 0, LINKNAME_SIZE+1);
        readlink(pathInput, linkVal, LINKNAME_SIZE);
        strncpy(headerData.fields.linkname, linkVal, LINKNAME_SIZE);
    }
    else if (S_ISDIR(statInfo.st_mode))
    {
        sprintf(headerData.fields.size, "%011o", 0);
        headerData.fields.typeflag[0] = '5';
    }

    // ignoring dev number for now
    // common fields
    strcpy(headerData.fields.magic, "ustar");
    headerData.fields.version[0] = '0';
    headerData.fields.version[1] = '0';
    sprintf(headerData.fields.mode, "%07o", statInfo.st_mode & 0xFFF);
    sprintf(headerData.fields.mtime, "%011o", statInfo.st_mtime);
    sprintf(headerData.fields.chksum, "%07o", computeChecksum(headerData));

    // write header
    fseek(fout, *blockIndex, SEEK_SET);
    fwrite(headerData.bin, sizeof(char), BLOCK_SIZE, fout);
    (*blockIndex) += BLOCK_SIZE;
    return 1;
}

int writeGUID(struct stat statInfo, headerData_t *headerData, int sOpt)
{
    // WRITE GROUP AND USER INFO //

    // extract uid, guid
    struct passwd *s_passwd;
    struct group *s_group;
    s_passwd = getpwuid(statInfo.st_uid);
    s_group = getgrgid(statInfo.st_gid);
    
    // if uid is too long to fit in header, write it in binary form
    if (statInfo.st_uid >= 1 << 21)
    {
        if (sOpt)
            return 0;
        else
        {
            uint32_t uidnum = htonl(statInfo.st_uid);
            headerData->fields.uid[0] |= (1 << 7);
            headerData->fields.uid[UID_SIZE-4] = (uidnum & 0xFF);
            headerData->fields.uid[UID_SIZE-3] = ((uidnum >> 8) & 0xFF);
            headerData->fields.uid[UID_SIZE-2] = ((uidnum >> 16) & 0xFF);
            headerData->fields.uid[UID_SIZE-1] = ((uidnum >> 24) & 0xFF);
        }
    }
    else
        sprintf(headerData->fields.uid, "%07o", statInfo.st_uid);
    sprintf(headerData->fields.gid, "%07o", statInfo.st_gid);
    sprintf(headerData->fields.uname, "%s", s_passwd->pw_name);
    sprintf(headerData->fields.gname, "%s", s_group->gr_name);
    return 1;
}

int splitName(char *path, headerData_t *headerData)
{
    // SPLIT FULL PATH INTO NAME AND PREFIX SECTIONS //

    int pathLen = strlen(path);

    // skip if path len is too long for name and prefix
    if (pathLen > (NAME_SIZE+PREFIX_SIZE+1))
        return 0;

    // if we need to use prefix section
    if (pathLen > NAME_SIZE)
    {
        // find first / to split into prefix and name
        int i = pathLen-NAME_SIZE;
        while (i < pathLen && path[i] != '/')
            i++;

        // if we hit pathLen before a slash we skip
        if (i == pathLen)
            return 0;
        else
        {
            strcpy(headerData->fields.name, path+i+1);
            strncpy(headerData->fields.prefix, path, i);
        }
    }
    else
        strcpy(headerData->fields.name, path); 
    return 1;
}
