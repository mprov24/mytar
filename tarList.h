#include "mytar.h"

void getTimeStr(headerData_t headerData, char *timeStr);
char getTypeFlagChar(headerData_t *headerData);
void listVerbose(headerData_t headerData, long recordSize);
