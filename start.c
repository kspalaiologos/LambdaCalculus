
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

#include "lambda.h"

static usage(void) {
    return puts(
        "Lambda calculus interpreter.\n"
        "Copyright (C) 2018 Kamila \"Palaiologos\" Szewczyk\n"
        "\n"
        "  -n    Use call-by-name evaluation.\n"
        "  -v    Use call-by-value evaluation.\n"
        "  -h    Display this help message."
    );
}

main(argc, argv) int argc; char ** argv; {
    void (*eval)(struct term_t **) = evaldeep;
    char buf[STRING_MAX], * arg;
    struct term_t * t;
    
    substart();
    
    while (*++argv) {
        arg = *argv;
        if (arg[0] != '-' || arg[1] == 0)
            continue;
        switch (arg[1]) {
            case 'h':
                usage();
                return 0;
            case 'v':
                eval = evalbvalue;
                continue;
            case 'n':
                eval = evalbname;
                continue;
            default:
                ;
        }
    }
    
    for (;;) {
        if (!fgets(buf, STRING_MAX, stdin) || strcmp(buf, ".\n") == 0)
            return 0;
        if (!(t = tparse(buf))) {
            puts("Parse error");
            continue;
        }
        eval(&t);
        tdparse(t, stdout);
        tfparse(t);
        putchar('\n');
    }
}
