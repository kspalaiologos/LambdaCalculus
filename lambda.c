
/* LambdaCalculus
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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "lambda.h"

#define EMPTY_BITMAP {{ 0, 0, 0, 0}}

struct bitmap {
    unsigned int t[4];
};

static struct term_t * palloc(void);
static char * nexttoken(char *, char * );
static char * nextparen(char *, struct term_t **);
static char * nextlambda(char *, struct term_t **);
static char * nextapp(char *, struct term_t **);
static void setbitmap(struct bitmap *, char);
static void clearbitmap(struct bitmap *, char);
static bool bitmapisset(const struct bitmap *, char);
static char bitmapfresh(const struct bitmap *, const struct bitmap *);
static void freevars(const struct term_t *, struct bitmap *, struct bitmap);
static void alpha(struct term_t *, char, char);
static void rec(struct term_t *, char, const struct term_t *, const struct bitmap *);

static const struct bitmap empty_set = EMPTY_BITMAP;
static const char var_set_str[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static struct bitmap var_set = EMPTY_BITMAP;

static struct term_t * palloc(void) {
    struct term_t * t = malloc(sizeof(*t));
    if (!t) {
        fputs("Out of memory.", stderr);
        abort();
    }
    return t;
}

static char * nexttoken(char * s, char * t) {
    while (isspace(*s))
        s++;
    *t = *s;
    return *s ? s + 1 : s;
}

static char * nextparen(char * s, struct term_t ** t) {
    char * p;
    char c;
    p = nexttoken(s, &c);
    if (c == 0)
        return s;
    else if (c == '(') {
        struct term_t * paren = NULL;
        p = nextapp(p, &paren);
        if (!paren) {
            puts("Invalid parenthesis content.");
            return s;
        }
        p = nexttoken(p, &c);
        if (c != ')') {
            puts("Parenthesis mismatch.");
            return s;
        }
        *t = paren;
        s = p;
    } else if (c == ')')
        return s;
    else {
        struct term_t * var = palloc();
        var->type = tvariabl;
        var->data.var = c;
        *t = var;
        s = p;
    }
    return s;
}

static char * nextlambda(char * s, struct term_t ** t) {
    struct term_t * lambda = NULL;
    char * p;
    char c;
    p = nexttoken(s, &c);
    if (c == '\\') {
        struct term_t * body = NULL;
        p = nexttoken(p, &c);
        if (c == 0) {
            puts("Unexpected end of input while reading variable.");
            goto finish;
        }
        if (c == '\\' || c == '(' || c == ')') {
            puts("Invalid varable name in lambda_tbda.");
            goto finish;
        }
        p = nextapp(p, &body);
        if (!body) {
            puts("Empty body found in lambda_tbda");
            goto finish;
        }
        lambda = palloc();
        lambda->type = tlambda;
        lambda->data.lambda_t.var = c;
        lambda->data.lambda_t.body = body;
        s = p;
    } else
        s = nextparen(s, &lambda);
finish:
    *t = lambda;
    return s;
}

static void setbitmap(struct bitmap * bmp, char c) {
    bmp->t[c/32] |= 1 << c%32;
}

static void clearbitmap(struct bitmap * bmp, char c) {
    bmp->t[c/32] &= ~(1 << c%32);
}

static bool bitmapisset(const struct bitmap * bmp, char c) {
    return bmp->t[c/32] & 1 << c%32;
}

static char bitmapfresh(const struct bitmap * set0, const struct bitmap * set1) {
    int i;
    char fresh = 0;
    for (i = 0; i < 4; i++) {
        unsigned int vars = var_set.t[i] & ~(set0->t[i] | set1->t[i]);
        if (vars) {
            int c = 0;
            if (!(vars & 0x0000FFFF)) c += 16;
            else vars &= 0x0000FFFF;
            if (!(vars & 0x00FF00FF)) c += 8;
            else vars &= 0x00FF00FF;
            if (!(vars & 0x0F0F0F0F)) c += 4;
            else vars &= 0x0F0F0F0F;
            if (!(vars & 0x33333333)) c += 2;
            else vars &= 0x33333333;
            if (!(vars & 0x55555555)) c += 1;
            fresh = i * 32 + c;
            break;
        }
    }
    if (!fresh) {
        puts("Running out of free variables.");
        fresh = '!';
    }
    return fresh;
}

static void freevars(const struct term_t * t, struct bitmap * free_vars, struct bitmap bound_vars) {
    const struct term_t * pterm = t;
    for (;;)
        switch (pterm->type) {
            case tlambda: {
                    setbitmap(&bound_vars, pterm->data.lambda_t.var);
                    pterm = pterm->data.lambda_t.body;
                    continue;
                }
            case tapp: {
                    freevars(pterm->data.app_t.left, free_vars, bound_vars);
                    pterm = pterm->data.app_t.right;
                    continue;
                }
            case tvariabl: {
                    if (bitmapisset(&bound_vars, pterm->data.var))
                        return;
                    setbitmap(free_vars, pterm->data.var);
                    return;
                }
            default:
                abort();
        }
}

static void alpha(struct term_t * t, char old, char new) {
    struct term_t * pterm = t;
    for (;;)
        switch (pterm->type) {
            case tlambda: {
                    if (pterm->data.lambda_t.var == old)
                        return;
                    pterm = pterm->data.lambda_t.body;
                    continue;
                }
            case tapp: {
                    alpha(pterm->data.app_t.left, old, new);
                    pterm = pterm->data.app_t.right;
                    continue;
                }
            case tvariabl: {
                    if (pterm->data.var == old)
                        pterm->data.var = new;
                    return;
                }
            default:
                abort();
        }
}

static void rec(struct term_t * t, char v, const struct term_t * s, const struct bitmap * free_vars) {
    struct term_t * pterm = t;
    for (;;)
        switch (pterm->type) {
            case tlambda: {
                    if (pterm->data.lambda_t.var == v)
                        return;
                    if (bitmapisset(free_vars, pterm->data.lambda_t.var)) {
                        register char u;
                        struct bitmap free_lambda_t = EMPTY_BITMAP;
                        freevars(pterm->data.lambda_t.body, &free_lambda_t, empty_set);
                        u = bitmapfresh(&free_lambda_t, free_vars);
                        alpha(pterm->data.lambda_t.body, pterm->data.lambda_t.var, u);
                        pterm->data.lambda_t.var = u;
                    }
                    pterm = pterm->data.lambda_t.body;
                    continue;
                }
            case tapp: {
                    rec(pterm->data.app_t.left, v, s, free_vars);
                    pterm = pterm->data.app_t.right;
                    continue;
                }
            case tvariabl: {
                    struct term_t * copy;
                    if (pterm->data.var != v)
                        return;
                    *pterm = *(copy = tcparse(s));
                    free(copy);
                    return;
                }
            default:
                abort();
        }
}

static bool isvalue(const struct term_t * pterm) {
    return pterm->type == tlambda || pterm->type == tvariabl;
}

static char * nextapp(char * s, struct term_t ** t) {
    struct term_t * left = NULL;
    s = nextlambda(s, &left);
    if (left) {
        while (*s) {
            struct term_t * right = NULL;
            s = nextlambda(s, &right);
            if (!right)
                goto invalid;
            else {
                struct term_t * top = palloc();
                top->type = tapp;
                top->data.app_t.left = left;
                top->data.app_t.right = right;
                left = top;
            }
        }
    }
invalid:
    *t = left;
    return s;
}

struct term_t * tparse(char * s) {
    struct term_t * t = NULL;
    nextapp(s, &t);
    return t;
}

struct term_t * tcparse(const struct term_t * t) {
    const struct term_t * pterm = t;
    struct term_t * const ret = palloc();
    struct term_t * p = ret;
    for (;;)
        switch ((p->type = pterm->type)) {
            case tlambda: {
                    p->data.lambda_t.var = pterm->data.lambda_t.var;
                    pterm = pterm->data.lambda_t.body;
                    p = p->data.lambda_t.body = palloc();
                    continue;
                }
            case tapp: {
                    p->data.app_t.left = tcparse(pterm->data.app_t.left);
                    pterm = pterm->data.app_t.right;
                    p = p->data.app_t.right = palloc();
                    continue;
                }
            case tvariabl: {
                    p->data.var = pterm->data.var;
                    return ret;
                }
            default:
                abort();
        }
}

void tfparse(struct term_t * t) {
    struct term_t * pterm = t;
    for (;;)
        switch (pterm->type) {
                struct term_t * tmp;
            case tlambda: {
                    tmp = pterm;
                    pterm = pterm->data.lambda_t.body;
                    free(tmp);
                    continue;
                }
            case tapp: {
                    tfparse(pterm->data.app_t.left);
                    tmp = pterm;
                    pterm = pterm->data.app_t.right;
                    free(tmp);
                    continue;
                }
            case tvariabl: {
                    free(pterm);
                    return;
                }
            default:
                abort();
        }
}

void tdparse(const struct term_t * t, FILE * stream) {
    const struct term_t * pterm = t;
    int nparen = 0;
    for (;;)
        switch (pterm->type) {
            case tlambda: {
                    fprintf(stream, "Lam (%c, ", pterm->data.lambda_t.var);
                    pterm = pterm->data.lambda_t.body;
                    nparen++;
                    continue;
                }
            case tapp: {
                    fputs("@ (", stream);
                    tdparse(pterm->data.app_t.left, stream);
                    fputs(", ", stream);
                    pterm = pterm->data.app_t.right;
                    nparen++;
                    continue;
                }
            case tvariabl: {
                    fputc(pterm->data.var, stream);
                    goto Close_paren;
                }
            default:
                abort();
        }
Close_paren:
    while (nparen--)
        fputc(')', stream);
}

void substart(void) {
    unsigned int i = 0;
    for (; i < sizeof(var_set_str) - 1; i++)
        setbitmap(&var_set, var_set_str[i]);
}

void substitute(struct term_t * t, char v, const struct term_t * s) {
    struct bitmap free_vars = EMPTY_BITMAP;
    freevars(s, &free_vars, empty_set);
    rec(t, v, s, &free_vars);
}

void evalbname(struct term_t ** ppterm) {
    struct term_t ** stack[STACK_SIZE] = { ppterm };
    int stackp = 0;
    while (stackp >= 0) {
        struct term_t * pterm = *stack[stackp];
        switch (pterm->type) {
            case tlambda: {
                    stackp--;
                    continue;
                }
            case tvariabl: {
                    return;
                }
            case tapp: {
                    if (pterm->data.app_t.left->type == tlambda) {
                        struct term_t * lambda_t = pterm->data.app_t.left;
                        substitute(lambda_t->data.lambda_t.body, lambda_t->data.lambda_t.var,
                                   pterm->data.app_t.right);
                        *stack[stackp] = lambda_t->data.lambda_t.body;
                        tfparse(pterm->data.app_t.right);
                        free(pterm);
                        free(lambda_t);
                        continue;
                    }
                    if (++stackp >= STACK_SIZE) {
                        puts("Stack overflow.");
                        return;
                    }
                    stack[stackp] = &pterm->data.app_t.left;
                    continue;
                }
            default:
                abort();
        }
    }
}

void evalbvalue(struct term_t ** ppterm) {
    struct term_t ** stack[STACK_SIZE] = { ppterm };
    int stackp = 0;
    while (stackp >= 0) {
        struct term_t * pterm = *stack[stackp];
        switch (pterm->type) {
            case tvariabl:
            case tlambda: {
                    stackp--;
                    continue;
                }
            case tapp: {
                    if (isvalue(pterm->data.app_t.right)) {
                        struct term_t * lambda_t = pterm->data.app_t.left;
                        if (lambda_t->type != tlambda) {
                            if (lambda_t->type == tvariabl)
                                return;
                            if (++stackp >= STACK_SIZE) {
                                puts("Stack overflow.");
                                return;
                            }
                            stack[stackp] = &pterm->data.app_t.left;
                            continue;
                        }
                        substitute(lambda_t->data.lambda_t.body, lambda_t->data.lambda_t.var,
                                   pterm->data.app_t.right);
                        *stack[stackp] = lambda_t->data.lambda_t.body;
                        tfparse(pterm->data.app_t.right);
                        free(pterm);
                        free(lambda_t);
                        continue;
                    }
                    if (++stackp >= STACK_SIZE) {
                        puts("Stack overflow.");
                        return;
                    }
                    stack[stackp] = &pterm->data.app_t.right;
                    continue;
                }
            default:
                abort();
        }
    }
}

void evaldeep(struct term_t ** ppterm) {
    while (1) {
        struct term_t * pterm = *ppterm;
        switch (pterm->type) {
            case tvariabl: {
                    return;
                }
            case tlambda: {
                    evaldeep(&pterm->data.lambda_t.body);
                    return;
                }
            case tapp: {
                    struct term_t * left;
                    evaldeep(&pterm->data.app_t.left);
                    left = pterm->data.app_t.left;
                    evaldeep(&pterm->data.app_t.right);
                    if (left->type == tlambda) {
                        evalbname(ppterm);
                        continue;
                    }
                    return;
                }
            default:
                abort();
        }
    }
}

