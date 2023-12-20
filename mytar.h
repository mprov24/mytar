#ifndef MYTAR_H
#define MYTAR_H

#include <stdio.h>
#include <stdint.h>

#include "tools.h"

#define NAME_OFFSET 0
#define MODE_OFFSET 100
#define UID_OFFSET 108
#define GID_OFFSET 116
#define SIZE_OFFSET 124
#define MTIME_OFFSET 136
#define CHKSUM_OFFSET 148
#define TYPEFLAG_OFFSET 156
#define LINKNAME_OFFSET 157
#define MAGIC_OFFSET 257
#define VERSION_OFFSET 263
#define UNAME_OFFSET 265
#define GNAME_OFFSET 297
#define DEVMAJOR_OFFSET 329
#define DEVMINOR_OFFSET 337
#define PREFIX_OFFSET 345

#define NAME_SIZE 100
#define MODE_SIZE 8
#define UID_SIZE 8
#define GID_SIZE 8
#define SIZE_SIZE 12
#define MTIME_SIZE 12
#define CHKSUM_SIZE 8
#define TYPEFLAG_SIZE 1
#define LINKNAME_SIZE 100
#define MAGIC_SIZE 6
#define VERSION_SIZE 2
#define UNAME_SIZE 32
#define GNAME_SIZE 32
#define DEVMAJOR_SIZE 8
#define DEVMINOR_SIZE 8
#define PREFIX_SIZE 155

#define BLOCK_SIZE 512


// required mode type
enum mainMode_t {create, listing, extract, NA}; 


struct __attribute__((__packed__)) s_headerFields
{
    char name[NAME_SIZE];
    char mode[MODE_SIZE];
    char uid[UID_SIZE];
    char gid[GID_SIZE];
    char size[SIZE_SIZE];
    char mtime[MTIME_SIZE];
    char chksum[CHKSUM_SIZE];
    char typeflag[TYPEFLAG_SIZE];
    char linkname[LINKNAME_SIZE];
    char magic[MAGIC_SIZE];
    char version[VERSION_SIZE];
    char uname[UNAME_SIZE];
    char gname[GNAME_SIZE];
    char devmajor[DEVMAJOR_SIZE];
    char devminor[DEVMINOR_SIZE];
    char prefix[PREFIX_SIZE];
};

union u_headerData
{
    struct s_headerFields fields;
    char bin[BLOCK_SIZE];
};

typedef union u_headerData headerData_t;

void handleArgs(int argc, char *argv[], enum mainMode_t *mainMode, 
                int *vOpt, int *sOpt, int *fOpt);
void getModes(char *modes, enum mainMode_t *mainMode, 
              int *vOpt, int *sOpt, int *fOpt);
int isInArgList(char *args[], int numArgs, char* fullName);
uint64_t computeChecksum(headerData_t headerData);
int verifyHeader(headerData_t headerData, int strict);
void combineName(headerData_t headerData, char *fullName);
void loopArchives(int argc, char *argv[], int vOpt, int sOpt, 
                  enum mainMode_t mainMode);

#endif
