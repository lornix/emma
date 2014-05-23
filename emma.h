/* emma - emma.h */

#ifndef emma_H
#define emma_H

#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* if VERSION + REVISION not set, give a default */
#ifndef VERREV
#define VERREV "0.0-devel"
#endif

/*
 * #define dbgprintf(x, ...)
 */
#define dbgprintf(x, ...) fprintf(stderr,x, ##__VA_ARGS__);

#define EXITERROR(x, ...) \
        do { \
            int saved_errno=errno; \
            fprintf(stderr,"%s:%d\n  ",__func__,__LINE__); \
            fprintf(stderr,x, ##__VA_ARGS__); \
            fprintf(stderr,"\n"); \
            if (saved_errno!=0) { \
              fprintf(stderr,"  %s\n",strerror(saved_errno)); \
            } \
            exit(1); \
        } while (0)

typedef struct {
    const char* filename;
} emmadat;

#endif
