#include "mytar.h"

void createPathDirs(char *path);
void extractFile(char *path, headerData_t headerData, int recordSize,
                 FILE *ftar, unsigned long ftarByte);
