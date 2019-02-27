
/* Black Pearl
 * Copyright (C) Kamila Palaiologos Szewczyk, 2019.
 * License: MIT
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
