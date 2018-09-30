
#include "lambda.h"

static usage(void) {
    puts(
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
