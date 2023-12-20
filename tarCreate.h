#include <sys/stat.h>
#include "mytar.h"

void writeData(FILE *fout, unsigned long *blockIndex, char *pathInput);
void executeCreate(int argc, char *argv[], int vOpt, int sOpt);
void recurseDirDFS(FILE *fout, unsigned long *blockIndex, char *pathInput, 
                    int vOpt, int sOpt);
void archivePath(FILE *fout, unsigned long *blockIndex, char *pathInput, 
                 int vOpt, int sOpt);
int writeHeader(FILE *fout, unsigned long *blockIndex, char *pathInput, 
                 struct stat statInfo, int sOpt);
int writeGUID(struct stat statInfo, headerData_t *headerData, int sOpt);
int splitName(char *path, headerData_t *headerData);
