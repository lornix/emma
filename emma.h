/* emma - emma.h */

#ifndef emma_H
#define emma_H

/* if VERSION + REVISION not set, give a default */
#ifndef VERREV
#define VERREV "0.0-devel"
#endif

typedef unsigned long int vma_t;

#define EXITERROR(x, ...) \
        do { \
            int saved_errno=errno; \
            fprintf(stderr,"%s:%d\n  ",__func__,__LINE__); \
            fprintf(stderr,x, ##__VA_ARGS__); \
            fprintf(stderr,"\n  %s\n",strerror(saved_errno)); \
            exit(1); \
        } while (0)


#endif
