#ifndef STRING_H
#define STRING_H

#include <stdint.h>

#define NULL (void*)0

uint32_t memncpy(char* dst, char* src, uint32_t size);
void memset(char* buff, char val, uint32_t size);
uint32_t strlen(const char* str);
char* strtok(char *s, const char *delim);
int strncmp(char* str1, char* str2, uint32_t size);
uint32_t strlen(const char* str);


#endif