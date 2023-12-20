#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "tools.h"

void *sMalloc(int size)
{
    void *p = malloc(size);
    if (!p)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

void *sCalloc(int items, int size)
{
    void *p = calloc(items, size);
    if (!p)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

void *sRealloc(void *inp, int size)
{
    void *outp = realloc(inp, size);
    if (!outp)
    {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    return outp;
}

void bail(char *msg)
{
    fprintf(stderr, msg);
    exit(EXIT_FAILURE);
}

void bailPerror(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

unsigned long int_pow(unsigned long base, unsigned long exp)
{
    unsigned long result = 1;
    while (exp)
    {
        if (exp % 2)
           result *= base;
        exp /= 2;
        base *= base;
    }
    return result;
}

void freeStrBuff(strBuff_t *buff)
{
    if (buff)
    {
        if (buff->p)
            free(buff->p);
        free(buff);
    }
}

strBuff_t *createStrBuff(int size)
{
    strBuff_t *str = sMalloc(sizeof(strBuff_t));
    /* Malloc and extra byte for null char */
    str->p = sMalloc((size+1) * sizeof(char));
    str->size = size + 1;

    (str->p)[0] = '\0';
    str->len = 0;
    return str;
}

void clearStrBuff(strBuff_t *str, int index)
{
    (str->p)[index] = '\0';
    str->len = index;
}

void appendToStrBuff(strBuff_t *str, char *snew)
{
    if (str && str->p)
    {
        int newlen = str->len + strlen(snew);
        /* if length + null_char == size of buffer*/
        if (str->len + strlen(snew) >= str->size-1)
        {
            str->p = sRealloc(str->p, str->size + MAX(newlen, BUFF_INC));
            str->size = (str->size) + MAX(newlen, BUFF_INC);
        }
        str->len = newlen;
        strcat(str->p, snew);
    }
    else
    {
        printf("Error: trying to append to null pointer");
        exit(EXIT_FAILURE);
    }
}
