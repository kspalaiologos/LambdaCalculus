#ifndef _LAMBDA_CALCULUS_H
#define _LAMBDA_CALCULUS_H

#include <stdio.h>

#define STACK_SIZE 1024
#define STRING_MAX 2048

enum {
    tlambda,
    tapp,
    tvariabl
};

struct term_t {
    int type;
    union {
        struct {
            char var;
            struct term_t * body;
        } lambda_t;
        struct {
            struct term_t * left;
            struct term_t * right;
        } app_t;
        char var;
    } data;
};

struct term_t * tparse(char *);
struct term_t * tcparse(const struct term_t *);
void tfparse(struct term_t *);
void tdparse(const struct term_t *, FILE *);

void substart(void);
void substitute(struct term_t *, char, const struct term_t *);

void evalbname(struct term_t **);
void evalbvalue(struct term_t **);
void evaldeep(struct term_t **);

#endif
