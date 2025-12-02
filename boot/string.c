#include "../include/string.h"

static char *g_saveptr = NULL;

static int is_delim(char ch, const char *delim)
{
    const char *d = delim;
    while (*d) {
        if (*d == ch) return 1;
        d++;
    }
    return 0;
}

char* strtok(char *s, const char *delim)
{
    char *start;

    if (s != NULL) start = s;
    else start = g_saveptr;
    

    if (start == NULL) return NULL;

    while (*start && is_delim(*start, delim)) start++;
    

    if (*start == '\0')
    {
        g_saveptr = NULL;
        return NULL;
    }

    char *token = start;

    while (*start && !is_delim(*start, delim)) start++;

    if (*start == '\0')
    {
        g_saveptr = NULL;
        return token;
    }

    *start = '\0';
    start++;

    g_saveptr = start;

    return token;
}



uint32_t memncpy(char* dst, char* src, uint32_t size)
{
    uint32_t i;
    for(i = 0; i < size; i++)
    {
        dst[i] = src[i];
    }
    return i;
}

void memset(char* buff, char val, uint32_t size)
{
    while(size)
    {
        buff[--size] = val;
    }
}

uint32_t strlen(const char* str)
{
    int len = 0;
    while(str[len])
    {
        len++;
    }
    return len;
}

int strncmp(char* str1, char* str2, uint32_t size)
{
    for(uint32_t i = 0; i < size; i++)
    {
        if(str1[i] != str2[i])
        {
            return str1[i] - str2[i];
        }
    }
    return 0;
}
