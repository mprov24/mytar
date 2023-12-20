#ifndef TOOLS_H
#define TOOLS_H

#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX(i, j) (((i) > (j)) ? (i) : (j))
#define BUFF_INC 15


void *sMalloc(int size);

void *sCalloc(int items, int size);

void *sRealloc(void *inp, int size);

void bail(char *msg);

void bailPerror(char *msg);

unsigned long int_pow(unsigned long base, unsigned long exp);


struct strBuff
{
    char *p;
    int size; /* size in memory */
    int len; /* length of string not including null char */
};

typedef struct strBuff strBuff_t;

void freeStrBuff(strBuff_t *buff);

strBuff_t *createStrBuff(int size);

void clearStrBuff(strBuff_t *str, int index);

void appendToStrBuff(strBuff_t *str, char *snew);

#endif
