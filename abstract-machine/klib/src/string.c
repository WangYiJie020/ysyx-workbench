#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    return 0;
  panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
    char* pd=dst;
    while(*src){
        *pd=*src;
        pd++;
        src++;
    }
    *pd=0;
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
    char* pd=dst;
    while(*pd)pd++;
    while(*src){
        *pd=*src;
        src++;
    }
    *pd=0;
    return dst;
}

int strcmp(const char *s1, const char *s2) {
    int tmp;
    while(*s1&&*s2){
         tmp = (int)*s1-(int)*s2;
         printf("%c - %c = %d\n",*s1,*s2,tmp);
         if(tmp)return tmp;
         s1++;
         s2++;
    }
    return (int)*s1-(int)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
    char* bs=(char*)s;
    char* es=bs+n;
    while(bs!=es){
        *bs=c;
        bs++;
    }
    return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const char* bs1=(const char*)s1;
    const char* bs2=(const char*)s2;
    const char* es1=bs1+n;
    while(bs1!=es1){
        int tmp=(*bs1-*bs2);
        if(tmp)return tmp;
    }
    return 0;
}

#endif
