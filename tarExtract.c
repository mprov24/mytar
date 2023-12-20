#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include "tarExtract.h"

void createPathDirs(char *path)
{
    // WHILE EXTRACTING, CREATE FULL PATH TO FILE IF IT DOESN'T EXIST YET //
    int pathLen = strlen(path);
    char tempDir[256];
    int i;

    struct stat sb;
    for (i=0;i<pathLen;i++)
    {
        // if we hit a directory, create it
        if (path[i] == '/')
        {
            strncpy(tempDir, path, i);
            tempDir[i] = '\0';
            if (lstat(tempDir, &sb)) // if we can't stat it, it must not exist
                mkdir(tempDir, 0777);
        }
    }
}

void extractFile(char *path, headerData_t headerData, int recordSize,
                 FILE *ftar, unsigned long ftarByte)
{
    fseek(ftar, ftarByte, SEEK_SET);
    char *buff = sMalloc(recordSize);

    mode_t mode = strtol(headerData.fields.mode, NULL, 8); 
    char typeflag = headerData.fields.typeflag[0];
    
    // create path to file if it doesn't exist yet
    createPathDirs(path);

    // if this is a reg file
    if (typeflag == '0' || typeflag == '\0')
    {
        FILE *fout = fopen(path, "w");
        if (!fout)
            bailPerror(path);
        // read full record from tarfile and write out
        if (fread(buff, 1, recordSize, ftar) != recordSize)
            bail("unexpected EOF\n");
        if (fwrite(buff, 1, recordSize, fout) != recordSize)
            bailPerror("fwrite");
        fclose(fout);
 
        // if anyone has execute permission, give everyone execute perm
        if (mode & (S_IXUSR | S_IXGRP| S_IXOTH))
        {
            if (chmod(path, 0777))
                fprintf(stderr, "cannot chmod: %s", path);
        }
        else
            if (chmod(path, 0666))
                fprintf(stderr, "cannot chmod: %s", path);
    }
    else if(typeflag == '2')
        if(symlink(headerData.fields.linkname, path))
            fprintf(stderr, "cannot create symlink: %s", path);
 
    free(buff);
}
